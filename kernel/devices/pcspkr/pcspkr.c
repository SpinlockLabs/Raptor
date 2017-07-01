#include <liblox/common.h>

#include <kernel/timer.h>

#include <kernel/arch/x86/io.h>

void pcspkr_note(int length, int freq) {
    uint32_t div = (uint32_t) (11931800 / freq);

    uint8_t t = 0;

    outb(0x43, 0xb6);
    outb(0x42, (uint8_t) div);
    outb(0x42, (uint8_t) (div >> 8));

    t = inb(0x61);
    outb(0x61, (uint8_t) (t | 0x3));

    ulong s, ss = 0;
    timer_get_relative_time(0, (ulong) length * 10, &s, &ss);

    t = (uint8_t) (inb(0x61) & 0xFC);
    outb(0x61, t);
}
