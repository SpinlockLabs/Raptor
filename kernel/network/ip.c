#include "ip.h"

#include <liblox/net.h>

uint16_t ipv4_calculate_checksum(struct ipv4_packet* p) {
    uint32_t sum = 0;
    uint16_t* s = (uint16_t*)p;

    for (int i = 0; i < 10; ++i) {
        sum += ntohs(s[i]);
    }

    if (sum > 0xFFFF) {
        sum = (sum >> 16) + (sum & 0xFFFF);
    }

    return ~(sum & 0xFFFF) & 0xFFFF;
}
