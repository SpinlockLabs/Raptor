#include "icmp.h"
#include "stack.h"
#include "log.h"
#include "config.h"

#include <liblox/net.h>

#include <kernel/dispatch/events.h>
#include <liblox/string.h>

static void ping_reply(
    network_iface_t* iface,
    uint32_t addr,
    uint16_t id,
    uint16_t seq,
    uint8_t* data,
    size_t data_size) {
    netconf_t* conf = netconf_get(iface);
    netconf_lock(conf);
    uint32_t source = conf->ipv4.source;
    netconf_unlock(conf);

    ipv4_packet_t* ipv4 = zalloc(
        sizeof(ipv4_packet_t) +
            sizeof(icmp4_packet_t) + data_size
    );
    icmp4_packet_t* pkt = (icmp4_packet_t*) ipv4->payload;
    pkt->type = ICMP_TYPE_ECHO_REPLY;
    pkt->code = 0;
    pkt->info.echo.id = id;
    pkt->info.echo.sequence = seq;
    memcpy(pkt->payload, data, data_size);
    icmp4_finalize(pkt);
    ipv4->protocol = IP_PROTOCOL_ICMP;
    ipv4->source = htonl(source);
    ipv4->destination = htonl(addr);

    size_t final = ipv4_finalize_packet(
        ipv4,
        sizeof(icmp4_packet_t) + data_size
    );

    network_stack_send_packet(
        iface,
        (uint8_t*) ipv4,
        final,
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

    netconf_t* conf = netconf_get(iface);
    netconf_lock(conf);

    if (htonl(ipv4->destination) != conf->ipv4.source) {
        netconf_unlock(conf);
        return;
    }
    netconf_unlock(conf);

    icmp4_packet_t* icmp = (icmp4_packet_t*) ipv4->payload;
    if (icmp->code != 0) {
        return;
    }

    if (icmp->type == ICMP_TYPE_ECHO) {
        ping_reply(
            iface,
            ntohl(ipv4->source),
            icmp->info.echo.id,
            icmp->info.echo.sequence,
            icmp->payload,
            ntohs(ipv4->length) - sizeof(icmp4_packet_t)
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
