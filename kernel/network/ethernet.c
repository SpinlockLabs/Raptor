#include "ethernet.h"

#include <liblox/memory.h>
#include <liblox/string.h>

ethernet_packet_t* ethernet_packet_create(
    uint8_t source_mac[6], uint8_t dest_mac[6],
    uint16_t type, uint8_t* buffer, size_t size
) {
    ethernet_packet_t* pkt = zalloc(sizeof(ethernet_packet_t) + size);
    pkt->type = type;
    pkt->source = source_mac;
    pkt->destination = dest_mac;
    memcpy(pkt->payload, buffer, size);
    return pkt;
}
