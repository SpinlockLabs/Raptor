#include "ip.h"

#include <liblox/net.h>
#include <liblox/memory.h>

uint16_t ip_calculate_checksum(void* p, size_t width) {
    uint32_t sum = 0;
    uint16_t* s = (uint16_t*) p;

    for (uint i = 0; i < width; ++i) {
        sum += ntohs(s[i]);
    }

    if (sum > 0xFFFF) {
        sum = (sum >> 16) + (sum & 0xFFFF);
    }

    return (uint16_t) (~(sum & 0xFFFF) & 0xFFFF);
}

udp_ipv4_packet_t* ipv4_create_udp_packet(size_t payload_size) {
    size_t total = sizeof(udp_ipv4_packet_t) + payload_size;
    udp_ipv4_packet_t* packet = zalloc(total);
    packet->ipv4.protocol = IP_PROTOCOL_UDP;
    packet->ipv4.length = htons((uint16_t) total);
    packet->udp.length = htons(sizeof(udp_packet_t) + payload_size);
    return packet;
}

size_t ipv4_finalize_packet(ipv4_packet_t* p, size_t payload_size) {
    if (p->length == 0) {
        if (p->protocol == IP_PROTOCOL_UDP) {
            p->length = htons(sizeof(udp_packet_t) + payload_size);
        } else {
            p->length = htons((uint16_t) payload_size);
        }
    }

    if (p->ident == 0) {
        p->ident = htons(1);
    }

    if (p->ttl == 0) {
        p->ttl = 0x40;
    }

    if (p->version_ihl == 0) {
        p->version_ihl = (0x4 << 4) | (0x5 << 0);
    }

    p->checksum = htons(ip_calculate_checksum(p, 10));

    return p->length;
}

bool ipv4_cidr_match(ipv4_address_cidr_t* cidr, ipv4_address_t* address) {
    if (cidr->mask == 0) {
        return true;
    }

    uint32_t netmask = htonl(cidr->address.address);
    uint32_t addr = htonl(address->address);
    uint8_t bits = cidr->mask;

    return !((addr ^ netmask) & htonl(0xFFFFFFFFu << (32 - bits)));
}
