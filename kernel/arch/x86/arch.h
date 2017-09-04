#pragma once

#include <stdint.h>

#if defined(ARCH_X64)
#include "arch-x64.h"
#else
#include "arch-x86.h"
#endif

typedef struct arch_process_extensions {
    page_directory_t* paging;
    cpu_registers_t registers;
} arch_process_extensions_t;

void set_kernel_stack(uintptr_t stack);

void syscall_handler(cpu_registers_t* r);
