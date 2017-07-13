#pragma once

#include <liblox/list.h>

#include <kernel/network/ip.h>

/* An IPv4 packet that is inmotion. */
typedef struct ipv4_packet_inmotion {
    ipv4_packet_t* packet;
    char* target_iface;
} ipv4_packet_inmotion_t;
