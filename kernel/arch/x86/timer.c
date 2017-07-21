#include <liblox/common.h>

#include <kernel/timer.h>

#include <kernel/cpu/task.h>
#include <kernel/entry.h>

#include "io.h"
#include "irq.h"

static ulong timer_ticks = 0;

static int timer_callback(cpu_registers_t* regs) {
    unused(regs);

    timer_ticks++;
    if (kernel_initialized) {
        ktask_queue_flush();
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

void timer_sleep(ulong ticks) {
    ulong start = timer_ticks;

    while ((timer_ticks - start) < ticks) {
        int_enable();
        asm("hlt");
    }
}

void (*lox_sleep_provider)(ulong ms) = timer_sleep;
