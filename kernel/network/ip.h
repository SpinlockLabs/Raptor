#pragma once

#include <stddef.h>
#include <stdint.h>

#include <liblox/common.h>

#define IPV4_PROTOCOL_UDP 17

struct ipv4_packet {
    uint8_t version_ihl;
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
} packed;

uint16_t ipv4_calculate_checksum(struct ipv4_packet* p);
