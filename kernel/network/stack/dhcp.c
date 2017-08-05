#include "dhcp.h"

#include <liblox/string.h>
#include <liblox/io.h>
#include <liblox/net.h>
#include <liblox/hashmap.h>

#include <kernel/network/dhcp.h>

#include <kernel/dispatch/events.h>
#include <kernel/network/ip.h>

#include "config.h"
#include "stack.h"
#include "log.h"

static dhcp_internal_state_t* get_state(network_iface_t* iface) {
    if (iface == NULL) {
        return NULL;
    }
    return hashmap_get(iface->manager_data, "dhcp");
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
    hashmap_set(iface->manager_data, "dhcp", state);

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

    info("Accepting offer for IP " L_IP_FMT " on interface %s\n",
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

    netconf_t* conf = netconf_get(iface);
    netconf_lock(conf);
    conf->ipv4.source = ntohl(state->offer);
    netconf_unlock(conf);
}

static void handle_potential_dhcp_reply(void* event, void* extra) {
    unused(extra);

    raw_ipv4_packet_t* pkt = event;

    network_iface_t* iface = network_iface_get(pkt->iface);
    if (iface == NULL) {
        return;
    }

    ipv4_packet_t* ipv4 = pkt->ipv4;

    if (ipv4->protocol != IP_PROTOCOL_UDP) {
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

    info("Interface %s received a DHCP offer for " L_IP_FMT "\n",
        iface->name, ip_cp(offer));

    dhcp_accept_offer(iface, offer);

    uint32_t gateway = 0;

    size_t i = sizeof(dhcp_packet_t);
    size_t j = 0;
    while (i < udp->length) {
        uint8_t type = dhcp->options[j];
        uint8_t len = dhcp->options[j + 1];
        uint8_t* data = &dhcp->options[j + 2];

        if (type == 255) {
            break;
        }

        if (type == 3) {
            gateway = *(uint32_t*)(data);
        }

        j += len + 2;
        i += len + 2;
    }

    dbg("Interface %s was told " L_IP_FMT " is the gateway\n",
         iface->name, ip_cp(gateway));

    netconf_t* conf = netconf_get(iface);
    netconf_lock(conf);
    conf->ipv4.gateway = ntohl(gateway);
    netconf_unlock(conf);

    event_dispatch(
        EVENT_NETWORK_STACK_IFACE_UPDATE,
        iface
    );
}

void network_stack_dhcp_init(void) {
    event_add_handler(
        EVENT_NETWORK_STACK_IPV4_PKT_RECEIVE,
        handle_potential_dhcp_reply,
        NULL
    );

    event_add_handler(
        EVENT_NETWORK_STACK_IFACE_UP,
        dhcp_handle_interface_up,
        NULL
    );

    event_add_handler(
        EVENT_NETWORK_STACK_IFACE_DOWN,
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
