#pragma once

#include <stddef.h>
#include <stdint.h>

#include <liblox/common.h>
#include <stdbool.h>

#include "udp.h"

#define IP_PROTOCOL_UDP 17
#define IP_PROTOCOL_ICMP 1

typedef union ipv4_address {
    struct {
        uint8_t a;
        uint8_t b;
        uint8_t c;
        uint8_t d;
    } packed;
    uint32_t address;
} ipv4_address_t;

typedef struct ipv4_address_cidr {
    ipv4_address_t address;
    uint8_t mask;
} ipv4_address_cidr_t;

typedef struct ipv4_packet {
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
} packed ipv4_packet_t;

typedef struct ip_packet_moving {
    union {
        ipv4_packet_t ipv4;
    };
    char* iface;
} packed ip_packet_moving_t;

typedef struct udp_ipv4_packet {
    ipv4_packet_t ipv4;
    udp_packet_t udp;
    uint8_t payload[];
} packed udp_ipv4_packet_t;

uint16_t ip_calculate_checksum(void* p, size_t width);
udp_ipv4_packet_t* ipv4_create_udp_packet(size_t payload_size);
size_t ipv4_finalize_packet(ipv4_packet_t* p, size_t payload_size);
bool ipv4_cidr_match(ipv4_address_cidr_t* cidr, ipv4_address_t* address);
uint32_t ipv4_address_parse(char*);
