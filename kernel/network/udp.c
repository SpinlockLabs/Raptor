#include <liblox/net.h>
#include "udp.h"

void udp_finalize_packet(udp_packet_t* packet, size_t payload_size) {
    packet->length = htons(sizeof(udp_packet_t) + payload_size);
}
