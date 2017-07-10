#pragma once

#include <liblox/common.h>

#include <stdint.h>

typedef struct ethernet_packet {
    /* Destination MAC address. */
    uint8_t destination[6];

    /* Source MAC address. */
    uint8_t source[6];

    /* Ethernet Type */
    uint16_t type;

    /* Payload */
    uint8_t payload[];
} packed ethernet_packet_t;

/*
  Allocates an ethernet packet on the heap.
  The given buffer is copied into the payload of the packet.
  You must manually free the packet after use.
*/
ethernet_packet_t* ethernet_packet_create(
    uint8_t source_mac[6], uint8_t dest_mac[6],
    uint16_t type, uint8_t* buffer, size_t size
);
