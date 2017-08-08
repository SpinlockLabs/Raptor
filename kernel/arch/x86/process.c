#include <liblox/memory.h>

#include <kernel/process/process.h>
#include <kernel/cpu/idle.h>

#include "paging.h"

#define KERNEL_STACK_SIZE 0x8000

void arch_process_init(process_t* proc) {
    proc->registers.esp = 0;
    proc->registers.eip = 0;
    proc->registers.ebp = 0;
}

void arch_process_init_kidle(process_t* proc) {
    proc->registers.eip = (uintptr_t) &cpu_run_idle;
    proc->registers.esp = (uintptr_t) malloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
    proc->registers.ebp = proc->registers.esp;
}

void arch_process_switch_next(void) {
    process_t* proc = process_get_next();
    uintptr_t esp = proc->registers.esp;
    uintptr_t ebp = proc->registers.ebp;
    uintptr_t eip = proc->registers.eip;

    page_directory_t* dir = proc->arch.paging;
    paging_switch_directory(dir);

    asm volatile (
        "mov %0, %%ebx\n"
        "mov %1, %%esp\n"
        "mov %2, %%ebp\n"
        "mov %3, %%cr3\n"
        "mov $0x10000, %%eax\n" /* read_eip() will return 0x10000 */
        "jmp *%%ebx"
        : : "r" (eip), "r" (esp), "r" (ebp), "r" (dir->physicalAddr)
        : "%ebx", "%esp", "%eax"
    );
}
