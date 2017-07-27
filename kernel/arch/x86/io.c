#include "io.h"

#if defined(__COMPCERT__)
#define DEFIO(prefix, type, prefix_el) \
    void out ## prefix(uint16_t port, type value) { \
        asm volatile("out" #prefix "%" #prefix_el"0, %w1" :: "a"(value) : "Nd"(port)); \
    } \
    type in ## prefix(uint16_t port) { \
        type ret;\
        asm volatile("in" #prefix " %w1, %" #prefix_el "0" : "=a"(ret) : "Nd"(port)); \
        return ret; \
    }

DEFINE_IO(b, uint8_t, b)
DEFINE_IO(s, uint16_t, s)
DEFINE_IO(l, uint32_t,)
#else
void outb(uint16_t port, uint8_t value) {
}

uint8_t inb(uint16_t port) {
    uint8_t value = 0;
    return value;
}

void outl(uint16_t port, uint32_t value) {
}

uint32_t inl(uint16_t port) {
    uint32_t value = 0;
    return value;
}

void outs(uint16_t port, uint16_t value) {
}

uint16_t ins(uint16_t port) {
    uint16_t value = 0;
    return value;
}
#endif

void io_wait(void) {
    asm volatile("jmp 1f\n\t"
                 "1:jmp 2f\n\t"
                 "2:");
}
