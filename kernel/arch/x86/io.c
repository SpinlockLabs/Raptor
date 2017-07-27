#include "io.h"

#if !defined(__COMPCERT__)
#define DEFIO(prefix, type, prefix_el) \
    void out ## prefix(uint16_t port, type value) { \
        asm volatile("out" #prefix "%" #prefix_el"0, %w1" :: "a"(value) : "Nd"(port)); \
    } \
    type in ## prefix(uint16_t port) { \
        type ret;\
        asm volatile("in" #prefix " %w1, %" #prefix_el "0" : "=a"(ret) : "Nd"(port)); \
        return ret; \
    }
#else
#define DEFIO(prefix, type, prefix_el) \
    void out ## prefix(uint16_t port, type value) { \
        asm volatile("out" #prefix "%" #prefix_el"0, %w1" :: "r"(value) : "r"(port)); \
    } \
    type in ## prefix(uint16_t port) { \
        type ret;\
        asm volatile("in" #prefix " %w1, %" #prefix_el "0" : "=r"(ret) : "r"(port)); \
        return ret; \
    }
#endif

DEFIO(b, uint8_t, b)
DEFIO(s, uint16_t, s)
DEFIO(l, uint32_t,)

void io_wait(void) {
    asm volatile("jmp 1f\n\t"
                 "1:jmp 2f\n\t"
                 "2:");
}
