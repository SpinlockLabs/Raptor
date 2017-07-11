#include "stack.h"
#include "iface.h"

#include <liblox/io.h>

#include <kernel/dispatch/events.h>

#define dbg(msg, ...) printf(DEBUG "[Network Stack] " msg, ##__VA_ARGS__)
#define info(msg, ...) printf(INFO "[Network Stack] " msg, ##__VA_ARGS__)

void network_stack_handle_interface_receive(
    network_iface_t* iface, uint8_t* buffer) {
    unused(buffer);

    char* name = iface->name;
    dbg("Interface %s received a packet.\n", name);
    network_packet_t pkt = {
        .interface = name,
        .buffer = buffer,
        .free = true
    };

    event_dispatch("network:stack:packet", &pkt);

    if (pkt.free) {
        free(pkt.buffer);
    }
}

void network_stack_on_interface_registered(void* event, void* extra) {
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
    event_register_handler(
        "network:iface:registered",
        network_stack_on_interface_registered,
        NULL
    );
}
