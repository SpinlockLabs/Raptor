#include <liblox/common.h>

#include <kernel/timer.h>

#include <stdbool.h>

#include "io.h"
#include "irq.h"

#define SUBTICKS_PER_TICK 1000

static ulong timer_ticks = 0;
static ulong timer_subticks = 0;

static int timer_callback(regs_t *regs) {
    unused(regs);

    if (++timer_subticks == SUBTICKS_PER_TICK) {
        timer_ticks++;
        timer_subticks = 0;
    }

    return 1;
}

void timer_init(uint32_t freq) {
    // Register callback.
    irq_add_handler(IRQ0, &timer_callback);

    // Determine divisor.
    uint32_t div = 1193180 / freq;

    // Send command byte.
    outb(0x43, 0x36);

    // Split into low/high bytes.
    uint8_t l = (uint8_t) (div & 0xFF);
    uint8_t h = (uint8_t) ((div >> 8) & 0xFF);

    // Send low/high bytes to PIT.
    outb(0x40, l);
    outb(0x40, h);
}

ulong timer_get_ticks(void) {
    return timer_ticks;
}

void timer_get_relative_time(ulong seconds, ulong subseconds, ulong *out_seconds,
                             ulong *out_subseconds) {
    if (subseconds + timer_subticks > SUBTICKS_PER_TICK) {
        *out_seconds = timer_ticks + seconds + 1;
        *out_subseconds = (subseconds + timer_subticks) - SUBTICKS_PER_TICK;
    } else {
        *out_seconds = timer_ticks + seconds;
        *out_subseconds = timer_subticks + subseconds;
    }
}
