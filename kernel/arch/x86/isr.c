#include <liblox/string.h>

#include <kernel/panic.h>

#include "isr.h"

static irq_handler_t isr_routines[256] = { 0 };

static char* exceptions[32] = {
    "Division by zero",
    "Debug",
    "NMI Interrupt",
    "Breakpoint",
    "Overflow",
    "Range Exceeded",
    "Invalid Opcode",
    "No Math Coprocessor",
    "Double Fault",
    "Coprocessor Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Segment Fault",
    "General Protection",
    "Page Fault",
    "Reserved",
    "Floating-Point Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_add_handler(size_t isr, irq_handler_t handler) {
    isr_routines[isr] = handler;
}

void isr_remove_handler(size_t isr) {
    isr_routines[isr] = 0;
}

void isr_init(void) {
}

used void fault_handler(cpu_registers_t* r) {
    int i = r->int_no;
    irq_handler_t handler = isr_routines[i];

    if (handler) {
        handler(r);
    } else {
        puts("[PANIC] Unhandled exception: ");

        puts(exceptions[i]);
        putc('\n');

        print_registers(r);

        panic(NULL);
    }
}

void print_registers(cpu_registers_t* r) {
    char buf[64] = {0};

#define _PUTRV(name, reg) \
        puts(name " = 0x"); \
        itoa(reg, buf, 16); \
        puts(buf); \
        putc('\n')

    _PUTRV("cs", r->cs);
    _PUTRV("ds", r->ds);
    _PUTRV("es", r->es);
    _PUTRV("fs", r->fs);
    _PUTRV("gs", r->gs);

    _PUTRV("eip", r->eip);
    _PUTRV("ebp", r->ebp);
    _PUTRV("edi", r->edi);
    _PUTRV("esp", r->esp);
    _PUTRV("esi", r->esi);

    _PUTRV("eax", r->eax);
    _PUTRV("ebx", r->ebx);
    _PUTRV("ecx", r->ecx);
    _PUTRV("edx", r->edx);

    _PUTRV("eflags", r->eflags);
    _PUTRV("int_no", r->int_no);
    _PUTRV("err_code", r->err_code);

#undef _PUTRV
}
