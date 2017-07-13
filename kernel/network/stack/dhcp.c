#include "dhcp.h"

#include <liblox/io.h>
#include <liblox/string.h>
#include <liblox/net.h>
#include <liblox/hashmap.h>

#include <kernel/network/dhcp.h>

#include <kernel/dispatch/events.h>
#include <kernel/network/ip.h>

#include "stack.h"
#include "log.h"

static dhcp_internal_state_t* get_state(network_iface_t* iface) {
    if (iface == NULL) {
        return NULL;
    }
    return hashmap_get(iface->_stack, "dhcp");
}

static ipv4_address_t dhcp_request_src = {{0, 0, 0, 0}};
static ipv4_address_t dhcp_request_dest = {{255, 255, 255, 255}};

static void dhcp_send(network_iface_t* iface, uint8_t* opts, size_t optsize) {
    size_t payload_size = sizeof(dhcp_packet_t) + optsize;
    udp_ipv4_packet_t *pkt = ipv4_create_udp_packet(payload_size);

    pkt->ipv4.source = htonl(dhcp_request_src.address);
    pkt->ipv4.destination = htonl(dhcp_request_dest.address);
    pkt->udp.source_port = htons(68);
    pkt->udp.destination_port = htons(67);

    dhcp_create_generic_request(iface, (dhcp_packet_t*) pkt->payload);
    memcpy(
        &pkt->payload[sizeof(dhcp_packet_t)],
        opts,
        optsize
    );
    udp_finalize_packet(&pkt->udp, payload_size);
    size_t total_size = ipv4_finalize_packet(&pkt->ipv4, payload_size);

    network_stack_send_packet(
        iface,
        (uint8_t*) pkt,
        total_size,
        PACKET_CLASS_IPV4,
        0
    );
}

static void dhcp_handle_interface_up(void* event, void* extra) {
    unused(extra);

    network_iface_t* iface = event;

    dhcp_internal_state_t* state = zalloc(sizeof(dhcp_internal_state_t));
    hashmap_set(iface->_stack, "dhcp", state);

    dhcp_send_request(iface);
}

static void dhcp_handle_interface_down(void* event, void* extra) {
    unused(extra);

    network_iface_t* iface = event;
    dhcp_internal_state_t* state = get_state(iface);
    if (state != NULL) {
        free(state);
    }
}

static void dhcp_accept_offer(network_iface_t* iface, uint32_t offer) {
    dhcp_internal_state_t* state = get_state(iface);
    state->offer = offer;

    uint8_t* ip = ((uint8_t*) &offer);

    info("Accepting offer for IP %d.%d.%d.%d on interface %s\n",
        ip[0], ip[1], ip[2], ip[3], iface->name);

    uint8_t options[] = {
        53,
        1,
        3,
        50,
        4,
        ip[0], ip[1], ip[2], ip[3],
        55,
        2,
        3,
        6,
        255
    };

    dhcp_send(iface, options, sizeof(options));

    state->accepted = true;
}

static void handle_potential_dhcp_reply(void* event, void* extra) {
    unused(extra);

    raw_ipv4_packet_t* pkt = event;

    network_iface_t* iface = network_iface_get(pkt->iface);
    if (iface == NULL) {
        return;
    }

    ipv4_packet_t* ipv4 = pkt->ipv4;
    if (ipv4->destination != dhcp_request_dest.address) {
        return;
    }

    if (ipv4->protocol != IPV4_PROTOCOL_UDP) {
        return;
    }
    udp_packet_t* udp = (udp_packet_t*) pkt->ipv4->payload;

    if (ntohs(udp->destination_port) != 68) {
        return;
    }

    dhcp_packet_t* dhcp = (dhcp_packet_t*) udp->payload;

    if (ntohl(dhcp->magic) != DHCP_MAGIC) {
        return;
    }

    dhcp_internal_state_t* state = get_state(iface);
    if (state->accepted) {
        return;
    }

    uint32_t offer = dhcp->yiaddr;

    if (offer == 0) {
        return;
    }

    uint8_t* offer8 = ((uint8_t*) &offer);
    info("Interface %s received a DHCP offer for %d.%d.%d.%d\n",
        iface->name, offer8[0], offer8[1], offer8[2], offer8[3]);

    dhcp_accept_offer(iface, offer);
}

void network_stack_dhcp_init(void) {
    event_add_handler(
        "network:stack:ipv4:packet-receive",
        handle_potential_dhcp_reply,
        NULL
    );

    event_add_handler(
        "network:stack:iface-up",
        dhcp_handle_interface_up,
        NULL
    );

    event_add_handler(
        "network:stack:iface-down",
        dhcp_handle_interface_down,
        NULL
    );
}

void dhcp_send_request(network_iface_t* iface) {
    dbg("Sending DHCP discovery request on interface %s...\n", iface->name);

    uint8_t options[] = {
        53, /* Message Type */
        1, /* Length */
        1,
        55,
        2,
        3,
        6,
        255 /* End */
    };

    dhcp_send(iface, options, sizeof(options));
}
