#pragma once

#include <stdint.h>

#if defined(ARCH_X64)
#include "arch-x64.h"
#else
#include "arch-x86.h"
#endif

typedef struct arch_process_extensions {
    void* _unused;
} arch_process_extensions_t;
