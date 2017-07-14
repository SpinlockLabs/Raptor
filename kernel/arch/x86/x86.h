#pragma once

#include <stdint.h>
#include <cpuid.h>

#include <liblox/io.h>

#include <kernel/panic.h>

#include "arch.h"
#include "cmdline.h"

#define cpu_equals(name) __builtin_cpu_is(name)
#define cpu_supports(feature) __builtin_cpu_supports(feature)
#define get_cpuid(in, a, b, c, d) __get_cpuid(in, a, b, c, d)

#define IRQ_CHAIN_SIZE 16
#define IRQ_CHAIN_DEPTH 4

typedef void (*irq_handler_t)(cpu_registers_t*);
typedef int (*irq_handler_chain_t)(cpu_registers_t*);

#define breakpoint(name) \
    if (cmdline_bool_flag("break-" name)) {\
      puts("[BREAK] " name "\n"); \
      panic(NULL); \
    }
