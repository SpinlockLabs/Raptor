#include "icmp.h"
#include "ip.h"

void icmp4_finalize(icmp4_packet_t* pkt) {
    pkt->checksum = ip_calculate_checksum(pkt, 4);
}
