#pragma once

#include <liblox/common.h>

#include <stddef.h>
#include <stdint.h>

#define ICMP_TYPE_ECHO_REPLY 0
#define ICMP_TYPE_ECHO 8

typedef struct icmp4_packet {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;

    union {
        struct {
            uint16_t id;
            uint16_t sequence;
        } echo;
        uint32_t gateway;
        struct {
            uint16_t _unused;
            uint16_t mtu;
        } fragment;
    } info;

    uint8_t payload[];
} packed icmp4_packet_t;

void icmp4_finalize(icmp4_packet_t*);
