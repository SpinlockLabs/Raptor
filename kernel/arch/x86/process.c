#include <liblox/memory.h>

#include <kernel/process/process.h>
#include <kernel/cpu/idle.h>
#include <kernel/paging.h>

#include "paging.h"
#include "elf32load.h"
#include "irq.h"

#define KERNEL_STACK_SIZE 0x8000

static uintptr_t frozen_stack;

extern uintptr_t initial_esp;

void sysexec(
    tty_t* tty,
    char* name,
    uint8_t* bytes,
    size_t size,
    int argc,
    char** argv) {
    int_disable();

    process_t* proc = zalloc(sizeof(process_t));
    proc->tty = tty;
    proc->name = name;
    proc->node = process_get_tree()->root;
    proc->pid = process_get_next_pid();
    proc->cmdline = argv;
    proc->image.entry = 0;
    proc->image.stack = initial_esp + 1;
    proc->status = PROCESS_LOADING;
    list_add(process_get_all(), proc);

    process_set_current(proc);
    page_directory_t* dir = paging_clone_directory(paging_get_directory());
    proc->arch.paging = dir;
    paging_switch_directory(dir);

    int envc = 0;
    char* env[] = {};

    bool result = elf32_execute(
        bytes,
        size,
        argc,
        argv,
        envc,
        env
    );

    if (result == false) {
        printf(ERROR "[ELF] Failed to execute process.\n");
    }
}

void arch_process_init_kidle(process_t* proc) {
    proc->arch.registers.eip = (uintptr_t) &cpu_run_idle;
    proc->arch.registers.esp = (uintptr_t) malloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
    proc->arch.registers.ebp = proc->arch.registers.esp;
    proc->image.stack = proc->arch.registers.esp;

    extern page_directory_t* kernel_directory;
    proc->arch.paging = kernel_directory;
}

extern uintptr_t read_eip(void);

void scheduler_switch_next(void) {
    process_t* process = process_get_next_ready();
    uintptr_t esp = process->arch.registers.esp;
    uintptr_t ebp = process->arch.registers.ebp;
    uintptr_t eip = process->arch.registers.eip;

    process_set_current(process);

    process->status = PROCESS_RUNNING;

    page_directory_t* dir = process->arch.paging;
    paging_switch_directory(dir);
    set_kernel_stack(process->image.stack);

    asm volatile (
        "mov %0, %%ebx\n"
        "mov %1, %%esp\n"
        "mov %2, %%ebp\n"
        "mov %3, %%cr3\n"
        "mov $0x10000, %%eax\n"
        "jmp *%%ebx"
        : : "r" (eip), "r" (esp), "r" (ebp), "r" (dir->physicalAddr)
        : "%ebx", "%esp", "%eax"
    );
}

void scheduler_switch_task(bool reschedule) {
    process_t* process = process_get_current();
    if (process == NULL) {
        return;
    }

    if (process->status != PROCESS_RUNNING) {
        scheduler_switch_next();
        return;
    }

    uintptr_t esp, ebp, eip;
    asm volatile ("mov %%esp, %0" : "=r" (esp));
    asm volatile ("mov %%ebp, %0" : "=r" (ebp));
    eip = read_eip();

    if (eip == 0x10000) {
        return;
    }

    process->arch.registers.esp = esp;
    process->arch.registers.ebp = ebp;
    process->arch.registers.eip = eip;

    if (reschedule && process != process_get_kidle()) {
        process_enqueue(process);
    }

    scheduler_switch_next();
}

void scheduler_init(void) {
    frozen_stack = (uintptr_t) valloc(KERNEL_STACK_SIZE);
}
