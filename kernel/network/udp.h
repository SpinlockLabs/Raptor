#pragma once

#include <stddef.h>
#include <stdint.h>

#include <liblox/common.h>

typedef struct udp_packet {
    uint16_t source_port;
    uint16_t destination_port;
    uint16_t length;
    uint16_t checksum;
    uint8_t payload[];
} packed udp_packet_t;

void udp_finalize_packet(udp_packet_t* packet, size_t payload_size);
