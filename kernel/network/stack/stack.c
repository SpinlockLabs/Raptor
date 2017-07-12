#include "stack.h"

#include <kernel/dispatch/events.h>

#include "ethernet.h"
#include "dhcp.h"
#include "log.h"

static void network_stack_handle_interface_receive(
    network_iface_t* iface, uint8_t* buffer) {
    unused(buffer);

    char* name = iface->name;
    raw_packet_t pkt = {
        .iface = name,
        .buffer = buffer,
        .free = true,
        .iface_class_type = iface->class_type
    };

    event_dispatch("network:stack:raw:packet", &pkt);

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

    iface->handle_receive = network_stack_handle_interface_receive;
}

void network_stack_init(void) {
    network_stack_ethernet_init();
    network_stack_dhcp_init();

    event_register_handler(
        "network:iface:registered",
        network_stack_on_interface_registered,
        NULL
    );
}
