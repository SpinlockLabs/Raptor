#include "icmp.h"
#include "stack.h"

#include <liblox/hashmap.h>
#include <liblox/net.h>

#include <kernel/dispatch/events.h>

static void ping_reply(network_iface_t* iface, uint32_t addr, uint16_t id, uint16_t seq) {
    uint32_t source = (uint32_t) hashmap_get(iface->_stack, "source");

    ipv4_packet_t* ipv4 = zalloc(sizeof(ipv4_packet_t) + sizeof(icmp4_packet_t));
    icmp4_packet_t* pkt = (icmp4_packet_t*) ipv4->payload;
    pkt->type = 0;
    pkt->code = ICMP_TYPE_ECHO_REPLY;
    pkt->data.echo.id = id;
    pkt->data.echo.sequence = seq;
    icmp4_finalize(pkt);
    ipv4->protocol = IP_PROTOCOL_ICMP;
    ipv4->length = htons((uint16_t) sizeof(*ipv4) + sizeof(*pkt));
    ipv4->source = htonl(source);
    ipv4->destination = htonl(addr);

    ipv4_finalize_packet(ipv4, sizeof(icmp4_packet_t));

    network_stack_send_packet(
      iface,
      (uint8_t*) ipv4,
      sizeof(*ipv4),
      PACKET_CLASS_IPV4,
      0
    );
}

static void handle_ipv4_packet(void* event, void* extra) {
    unused(extra);

    raw_ipv4_packet_t* pkt = event;
    char* iface_name = pkt->iface;
    network_iface_t* iface = network_iface_get(iface_name);

    if (iface == NULL) {
        return;
    }

    ipv4_packet_t* ipv4 = pkt->ipv4;
    if (ipv4->protocol != IP_PROTOCOL_ICMP) {
        return;
    }

    uint32_t source = (uint32_t) hashmap_get(iface->_stack, "source");

    if (htonl(ipv4->destination) != source) {
        return;
    }

    icmp4_packet_t* icmp = (icmp4_packet_t*) ipv4->payload;
    if (icmp->type != 0) {
        return;
    }

    if (icmp->code == ICMP_TYPE_ECHO) {
        ping_reply(
            iface,
            ntohl(ipv4->source),
            icmp->data.echo.id,
            icmp->data.echo.sequence
        );
    }
}

void network_stack_icmp_init(void) {
    event_add_handler(
        "network:stack:ipv4:packet-receive",
        handle_ipv4_packet,
        NULL
    );
}
