#pragma once

#include <liblox/common.h>

struct ethernet_packet {
    uint8_t destination[6];
    uint8_t source[6];
    uint16_t type;
    uint8_t payload[];
} packed;
