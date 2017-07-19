#include "stack.h"

#include <liblox/hashmap.h>
#include <kernel/dispatch/events.h>

#include "arp.h"
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
        event_dispatch("network:stack:ipv4:packet-receive", &ipv4);
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

static void network_stack_handle_interface_receive(
    network_iface_t* iface, uint8_t* buffer) {
    unused(buffer);

    char* name = iface->name;
    raw_packet_t pkt = {
        .direction = PACKET_DIRECTION_IN,
        .iface = name,
        .buffer = buffer,
        .free = true,
        .iface_class_type = iface->class_type,
        .length = 0,
        .translated = false
    };

    event_dispatch("network:stack:raw:packet-receive", &pkt);

    if (!pkt.translated) {
        network_stack_handle_untranslated_receive(&pkt);
    }

    if (pkt.free) {
        free(pkt.buffer);
    }
}

static void network_stack_on_interface_registered(void* event, void* extra) {
    unused(extra);

    char* name = event;
    network_iface_t* iface = network_iface_get(name);

    if (iface == NULL) {
        return;
    }

    info("Interface %s registered.\n", name);

    iface->_stack = hashmap_create(2);
    iface->handle_receive = network_stack_handle_interface_receive;

    event_dispatch("network:stack:iface-up", iface);
}

static void network_stack_on_interface_destroying(void* event, void* extra) {
    unused(extra);

    network_iface_t* iface = event;

    event_dispatch("network:stack:iface-down", iface);

    if (iface->_stack != NULL) {
        hashmap_free((hashmap_t*) iface->_stack);
        iface->_stack = NULL;
    }

    info("Interface %s destroyed.\n", iface->name);
}

void network_stack_send_packet(
    network_iface_t* iface,
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

    event_dispatch("network:stack:raw:packet-send", &pkt);

    if (!pkt.translated) {
        network_stack_handle_untranslated_send(&pkt);
    }

    if (pkt.free) {
        free(pkt.buffer);
    }
}

void network_stack_init(void) {
    network_stack_arp_init();
    network_stack_icmp_init();
    network_stack_route_init();
    network_stack_ethernet_init();
    network_stack_dhcp_init();

    event_add_handler(
        "network:iface:registered",
        network_stack_on_interface_registered,
        NULL
    );

    event_add_handler(
        "network:iface:destroying",
        network_stack_on_interface_destroying,
        NULL
    );
}
