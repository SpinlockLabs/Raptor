#include "arch.h"

#include <liblox/io.h>

void syscall_handler(cpu_registers_t* r) {
    uintptr_t id = r->eax;
    uintptr_t args = r->ebx;
    printf(DEBUG "System Call %d (args = 0x%x)\n", id, args);
}
