/**
 * The network stack ethernet layer translates ethernet packets
 * into a standard network stack packet.
 */
#include "ethernet.h"
#include "stack.h"
#include "arp.h"
#include "config.h"
#include "log.h"

#include <liblox/net.h>
#include <liblox/string.h>

#include <kernel/network/ethernet.h>
#include <kernel/dispatch/events.h>

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

    if (ntohs(packet->type) == ETH_TYPE_IPV4) {
        return true;
    }

    return false;
}

static void handle_ethernet_packet_received(void* event, void* extra) {
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
        "network:stack:ipv4:packet-receive",
        &real_pkt
    );
}

static void handle_ethernet_packet_send(void* event, void* extra) {
    unused(extra);

    raw_packet_t* out = event;

    if (out->iface_class_type != IFACE_CLASS_ETHERNET) {
        return;
    }

    network_iface_t* iface = network_iface_get(out->iface);
    if (iface == NULL) {
        return;
    }

    uint16_t ether_type = 0;

    if (out->packet_class == PACKET_CLASS_IPV4) {
        ether_type = ETH_TYPE_IPV4;
    } else if (out->packet_class == PACKET_CLASS_ARP) {
        ether_type = ETH_TYPE_ARP;
    } else {
        return;
    }

    uint8_t dest[6] = {255, 255, 255, 255, 255, 255};

    netconf_t* conf = netconf_get(iface);
    netconf_lock(conf);
    uint32_t gw = conf->ipv4.gateway;
    netconf_unlock(conf);
    arp_lookup(iface, gw, dest);

    uint8_t mac[6] = {0};
    network_iface_get_mac(iface, mac);
    size_t payload_size = out->length;
    size_t total_size = sizeof(ethernet_packet_t) + payload_size;
    ethernet_packet_t* ether = zalloc(total_size);
    ether->type = htons(ether_type);
    memcpy(&ether->source, mac, 6);
    memcpy(&ether->destination, dest, 6);
    memcpy(&ether->payload, out->buffer, payload_size);

    network_iface_error_t error = network_iface_send(
        iface,
        (uint8_t*) ether,
        total_size
    );

    if (error != IFACE_ERR_OK) {
        dbg(
            "Failed to send ethernet packet on interface %s (error = %d)\n",
            iface->name,
            error
        );
    }
}

void network_stack_ethernet_init(void) {
    event_add_handler(
        "network:stack:raw:packet-receive",
        handle_ethernet_packet_received,
        NULL
    );

    event_add_handler(
        "network:stack:raw:packet-send",
        handle_ethernet_packet_send,
        NULL
    );
}
