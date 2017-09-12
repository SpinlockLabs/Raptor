#pragma once

#include <stdint.h>

#include <liblox/io.h>

#include <kernel/panic.h>

#include "arch.h"
#include "cmdline.h"

#define IRQ_CHAIN_SIZE 16
#define IRQ_CHAIN_DEPTH 16

typedef void (*irq_handler_t)(cpu_registers_t*);
typedef int (*irq_handler_chain_t)(cpu_registers_t*);

static inline uint64_t arch_x86_get_timestamp(void) {
#if defined(__COMPCERT__)
    return 0;
#endif

    unsigned long tsc_low, tsc_high;
    asm volatile("rdtsc"  : "=d"(tsc_high), "=a" (tsc_low));
    return ((uint64_t) tsc_high << 32) | tsc_low;
}
