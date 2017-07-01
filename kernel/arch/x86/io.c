#include "io.h"

inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %%dx, %%eax" : "=a" (ret) : "dN" (port));
    return ret;
}

inline void outl(uint16_t port, uint32_t value) {
    asm volatile ("outl %%eax, %%dx" : : "dN" (port), "a" (value));
}

inline void outs(uint16_t port, uint16_t value) {
    asm volatile ("inw %1, %0" : "=a" (value) : "dN" (port));
}

inline uint16_t ins(uint16_t port) {
   uint16_t ret;
   asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

inline void io_wait(void) {
    asm volatile("jmp 1f\n\t"
                 "1:jmp 2f\n\t"
                 "2:");
}
