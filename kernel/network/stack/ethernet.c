/**
 * The network stack ethernet layer translates ethernet packets
 * into a standard network stack packet.
 */
#include "ethernet.h"
#include "stack.h"

#include <kernel/network/ethernet.h>
#include <kernel/dispatch/events.h>
#include <liblox/io.h>

static bool is_ours(network_iface_t* iface, ethernet_packet_t* packet) {
    uint8_t our_mac[6];
    network_iface_get_mac(iface, our_mac);
    uint8_t* dest = packet->destination;

    if (dest[0] == 255 && dest[1] == 255 &&
        dest[2] == 255 && dest[3] == 255 &&
        dest[4] == 255 && dest[5] == 255) {
        return true;
    }

    for (uint8_t i = 0; i < 5; i++) {
        if (dest[i] != our_mac[i]) {
            return false;
        }
    }

    if (packet->type == ETH_TYPE_IPV4) {
        return true;
    }

    return false;
}

static void handle_ethernet_packet(void* event, void* extra) {
    unused(extra);

    raw_packet_t* raw = event;

    if (raw->iface_class_type != IFACE_CLASS_ETHERNET) {
        return;
    }

    uint8_t* buffer = raw->buffer;
    network_iface_t* iface = network_iface_get(raw->iface);
    if (iface == NULL) {
        return;
    }
    ethernet_packet_t* eth = (ethernet_packet_t*) buffer;

    if (!is_ours(iface, eth)) {
        return;
    }

    raw_ipv4_packet_t real_pkt = {
        .iface = raw->iface,
        .ipv4 = (ipv4_packet_t*) eth->payload
    };

    event_dispatch(
      "network:stack:ipv4:packet",
      &real_pkt
    );
}

void network_stack_ethernet_init(void) {
    event_register_handler(
        "network:stack:raw:packet",
        handle_ethernet_packet,
        NULL
    );
}
