#include <liblox/string.h>

#include <kernel/panic.h>

#include "isr.h"

void isr_add_handler(size_t isr, irq_handler_t handler) {
    isr_routines[isr] = handler;
}

void isr_rem_handler(size_t isr) {
    isr_routines[isr] = 0;
}

void isr_init(void) {
}

used void fault_handler(regs_t *r) {
    int i = r->int_no;
    irq_handler_t handler = isr_routines[i];

    if (handler) {
        handler(r);
    } else {
        puts("[PANIC] Unhandled exception: ");
        puts(exceptions[i]);
        putc('\n');

        char buf[64];

#define _PUTRV(name, reg) \
        puts(name " = 0x"); \
        itoa(reg, buf, 16); \
        puts(buf); \
        putc('\n')

        _PUTRV("cs", r->cs);
        _PUTRV("ds", r->ds);
        _PUTRV("fs", r->fs);
        _PUTRV("gs", r->gs);

        _PUTRV("eip", r->eip);
        _PUTRV("esp", r->esp);
        _PUTRV("esi", r->esi);
        _PUTRV("eax", r->eax);
#undef _PUTRV

        panic(NULL);
    }
}
