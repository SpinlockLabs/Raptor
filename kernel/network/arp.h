#pragma once

#include <liblox/common.h>

#include <stdint.h>

typedef struct arp_packet {
    uint16_t htype;
    uint16_t proto;

    uint8_t hlen;
    uint8_t plen;

    uint16_t oper;

    uint8_t sender_ha[6];
    uint32_t sender_ip;
    uint8_t target_ha[6];
    uint32_t target_ip;

    uint8_t padding[18];
} packed arp_packet_t;
