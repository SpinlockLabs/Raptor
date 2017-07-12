#pragma once

#include <stdint.h>

#include <liblox/common.h>

#include "iface.h"

#define DHCP_MAGIC 0x63825363

typedef struct dhcp_packet {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;

    uint32_t xid;

    uint16_t secs;
    uint16_t flags;

    uint32_t ciaddr;
    uint32_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;

    uint8_t chaddr[16];

    uint8_t sname[64];
    uint8_t file[128];

    uint32_t magic;

    uint8_t options[];
} packed dhcp_packet_t;

void dhcp_create_generic_request(network_iface_t* iface, dhcp_packet_t* dhcp);
