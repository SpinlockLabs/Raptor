#include <liblox/net.h>

#include <kernel/network/dhcp.h>

#include <kernel/dispatch/events.h>
#include <kernel/network/ip.h>
#include <liblox/io.h>

#include "stack.h"
#include "log.h"

static void handle_potential_dhcp(void* event, void* extra) {
    unused(extra);

    raw_ipv4_packet_t* pkt = event;
    ipv4_packet_t* ipv4 = pkt->ipv4;
    if (ipv4->destination != ipv4_address(255, 255, 255, 255)) {
        return;
    }

    if (ipv4->protocol != IPV4_PROTOCOL_UDP) {
        return;
    }
    udp_packet_t* udp = (udp_packet_t*) pkt->ipv4->payload;

    if (ntohs(udp->source_port) != 68) {
        return;
    }

    dhcp_packet_t* dhcp = (dhcp_packet_t*) udp->payload;

    if (ntohl(dhcp->magic) != DHCP_MAGIC) {
        return;
    }

    uint32_t offer = dhcp->yiaddr;

    if (offer == 0) {
        return;
    }

    uint8_t* offer8 = ((uint8_t*) &offer);
    info("Interface %s received DHCP offer for %d.%d.%d.%d\n",
        pkt->iface, offer8[0], offer8[1], offer8[2], offer8[3]);
}

void network_stack_dhcp_init(void) {
    event_register_handler(
        "network:stack:ipv4:packet",
        handle_potential_dhcp,
        NULL
    );
}
