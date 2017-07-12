#include "udp.h"

#include <liblox/net.h>

void udp_finalize_packet(udp_packet_t* packet, size_t payload_size) {
    if (packet->length == 0) {
        packet->length = htons(sizeof(udp_packet_t) + payload_size);
    }
}
