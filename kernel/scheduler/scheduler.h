/* A simple process scheduling system. */
#pragma once

#include <kernel/arch.h>

/* The scheduler state. */
typedef struct scheduler_state {
    cpu_registers_t registers;
} scheduler_state_t;
