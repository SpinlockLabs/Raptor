#include <liblox/common.h>
#include <kernel/timer.h>

#include "io.h"
#include "irq.h"

static uint32_t tick = 0;

static int timer_callback(regs_t *regs) {
    unused(regs);

    tick++;
    return 1;
}

void arch_x86_timer_init(uint32_t freq) {
    // Register callback.
    irq_add_handler(IRQ0, &timer_callback);

    // Determine divisor.
    uint32_t div = 1193180 / freq;

    // Send command byte.
    outb(0x43, 0x36);

    // Split into low/high bytes.
    uint8_t l = (uint8_t)(div & 0xFF);
    uint8_t h = (uint8_t)((div >> 8) & 0xFF);

    // Send low/high bytes to PIT.
    outb(0x40, l);
    outb(0x40, h);
}

uint32_t arch_x86_timer_get_ticks(void) {
    return tick;
}

uint32_t (*arch_timer_get_ticks)(void) = arch_x86_timer_get_ticks;
void (*arch_timer_init)(uint32_t) = arch_x86_timer_init;
