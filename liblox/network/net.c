#include "../net.h"

static inline uint16_t bswap_16(uint16_t x) {
    return x << 8 | x >> 8;
}

static inline uint32_t bswap_32(uint32_t x) {
    return x >> 24 | (x >> 8 & 0xff00) | (x << 8 & 0xff0000) | x << 24;
}

uint16_t htons(uint16_t n) {
    union { int i; char c; } u = {1};

    return u.c ? bswap_16(n) : n;
}

uint16_t ntohs(uint16_t n) {
    union { int i; char c; } u = {1};
    return u.c ? bswap_16(n) : n;
}

uint32_t htonl(uint32_t n) {
    union { int i; char c; } u = {1};
    return u.c ? bswap_32(n) : n;
}

uint32_t ntohl(uint32_t n) {
    union { int i; char c; } u = {1};
    return u.c ? bswap_32(n) : n;
}
