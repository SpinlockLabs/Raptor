#pragma once

#include "../common.h"

#include <stddef.h>
#include <stdint.h>

typedef struct trace {
    void* call;
} trace_t;

void backtrace(trace_t* traces, uint size);
