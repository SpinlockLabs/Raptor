#include "stack.h"

#include <liblox/string.h>
#include <liblox/hashmap.h>
#include <liblox/memory.h>

#include <kernel/dispatch/events.h>
#include <kernel/cpu/task.h>

#include "arp.h"
#include "config.h"
#include "icmp.h"
#include "ethernet.h"
#include "dhcp.h"
#include "route/layer.h"

#include "log.h"

static void network_stack_handle_untranslated_receive(
    raw_packet_t* pkt
) {
    if (pkt->packet_class == PACKET_CLASS_IPV4) {
        raw_ipv4_packet_t ipv4 = {
            .iface = pkt->iface,
            .raw = pkt,
            .ipv4 = (ipv4_packet_t*) pkt->buffer
        };
        event_dispatch(EVENT_NETWORK_STACK_IPV4_PKT_RECEIVE, &ipv4);
    }
}

static void network_stack_handle_untranslated_send(
    raw_packet_t* pkt
) {
    /**
     * TODO(kaendfinger): We don't want to unsafely send packets currently.
     * In the future, we could make this a network interface specific option.
     */
    unused(pkt);
}

static netif_error_t network_stack_handle_interface_receive(
    netif_t* iface, uint8_t* buffer, size_t size) {
    char* name = iface->name;
    raw_packet_t pkt = {
        .direction = PACKET_DIRECTION_IN,
        .iface = name,
        .buffer = buffer,
        .free = true,
        .iface_class_type = iface->class_type,
        .length = size,
        .translated = false
    };

    event_dispatch(EVENT_NETWORK_STACK_RAW_PKT_RECEIVE, &pkt);

    if (!pkt.translated) {
        network_stack_handle_untranslated_receive(&pkt);
    }

    if (pkt.free) {
        free(pkt.buffer);
    }

    return IFACE_ERR_OK;
}

static void network_stack_on_interface_registered(void* event, void* extra) {
    unused(extra);

    char* name = event;
    netif_t* iface = netif_get(name);

    if (iface == NULL) {
        return;
    }

    if (iface->flags.stackless) {
        return;
    }

    network_stack_takeover(iface);
}

static void network_stack_on_interface_destroying(void* event, void* extra) {
    unused(extra);

    netif_t* iface = event;

    if (iface->flags.stackless) {
        return;
    }

    network_stack_disown(iface);
}

void network_stack_send_packet(
    netif_t* iface,
    uint8_t* buffer,
    size_t size,
    packet_class_t packet_class,
    uint32_t flags) {
    raw_packet_t pkt = {
        .direction = PACKET_DIRECTION_OUT,
        .iface = iface->name,
        .buffer = buffer,
        .length = (uint16_t) size,
        .free = true,
        .iface_class_type = iface->class_type,
        .packet_class = packet_class,
        .flags = flags
    };

    event_dispatch(EVENT_NETWORK_STACK_RAW_PKT_SEND, &pkt);

    if (!pkt.translated) {
        network_stack_handle_untranslated_send(&pkt);
    }

    if (pkt.free) {
        free(pkt.buffer);
    }
}

void network_stack_init(void) {
    network_stack_config_init();
    network_stack_arp_init();
    network_stack_icmp_init();
    network_stack_route_init();
    network_stack_ethernet_init();
    network_stack_dhcp_init();

    event_add_handler(
        EVENT_NETWORK_IFACE_REGISTERED,
        network_stack_on_interface_registered,
        NULL
    );

    event_add_handler(
        EVENT_NETWORK_IFACE_DESTROYING,
        network_stack_on_interface_destroying,
        NULL
    );
}

bool network_stack_takeover(netif_t* iface) {
    if (iface->manager != NULL) {
        warn(
            "Failed to takeover interface %s: '%s' already owns it.\n",
            iface->name,
            iface->manager
        );
        return false;
    }

    dbg("Taking over interface %s.\n", iface->name);

    iface->manager = "stack";
    iface->manager_data = hashmap_create(2);
    iface->handle_receive = network_stack_handle_interface_receive;

    event_dispatch(EVENT_NETWORK_STACK_IFACE_UP, iface);
    return true;
}

bool network_stack_disown(netif_t* iface) {
    if (iface->manager == NULL ||
        strcmp(iface->manager, "stack") != 0) {
        return true;
    }

    event_dispatch(EVENT_NETWORK_STACK_IFACE_DOWN, iface);

    if (iface->manager_data != NULL) {
        hashmap_free((hashmap_t*) iface->manager_data);
        iface->manager_data = NULL;
    }

    dbg("Disowning interface %s.\n", iface->name);

    iface->manager = NULL;
    return true;
}

static void stack_takeover_later(void* iface) {
    network_stack_takeover(iface);
}

void network_stack_takeover_async(netif_t* iface) {
    ktask_queue(stack_takeover_later, iface);
}
