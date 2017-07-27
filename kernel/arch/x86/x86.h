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

#define breakpoint(name) \
    if (cmdline_bool_flag("break-" name)) {\
      puts("[BREAK] " name "\n"); \
      panic(NULL); \
    }
