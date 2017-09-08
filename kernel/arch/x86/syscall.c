#include "arch.h"

#include <kernel/syscall/table.h>
#include <kernel/process/process.h>

void syscall_handler(cpu_registers_t* r) {
    uintptr_t id = r->eax;
    uintptr_t args = r->ebx;

    syscall_result_t result = syscall_run(
      process_get_current()->callset,
      id,
      (uintptr_t*) args
    );

    r->eax = (uint32_t) result;
}
