#pragma once

#include <stddef.h>
#include <stdint.h>

#include <liblox/common.h>

struct udp_packet {
    uint16_t source_port;
    uint16_t destination_port;
    uint16_t length;
    uint16_t checksum;
    uint8_t payload[];
} packed;
