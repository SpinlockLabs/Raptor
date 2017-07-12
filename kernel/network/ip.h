#pragma once

#include <stddef.h>
#include <stdint.h>

#include <liblox/common.h>

#include "udp.h"

#define IPV4_PROTOCOL_UDP 17

typedef struct ipv4_packet {
    union {
        uint8_t version_ihl;

        struct {
            uint8_t version : 4;
            uint8_t ihl : 4;
        };
    };
    uint8_t dscp_ecn;
    uint16_t length;
    uint16_t ident;
    uint16_t flags_fragment;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t source;
    uint32_t destination;
    uint8_t payload[];
} packed ipv4_packet_t;

typedef struct udp_ipv4_packet {
    ipv4_packet_t ipv4;
    udp_packet_t udp;
    uint8_t payload[];
} packed udp_ipv4_packet_t;

uint16_t ipv4_calculate_checksum(ipv4_packet_t* p);
void ipv4_finalize_packet(ipv4_packet_t* p, size_t payload_size);

uint32_t ipv4_address(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
