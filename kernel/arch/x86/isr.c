#include "idt.h"
#include "isr.h"
#include "vga.h"

void isr_add_handler(size_t isr, irq_handler_t handler) {
    isr_routines[isr] = handler;
}

void isr_rem_handler(size_t isr) {
    isr_routines[isr] = 0;
}

void isr_init(void) {
}

void fault_handler(regs_t *r) {
    int i = r->int_no;
    irq_handler_t handler = isr_routines[i];

    if (handler) {
        handler(r);
    } else {
        vga_writestring("Unhandled exception ");
        vga_writestring(exceptions[i]);
        vga_writestring("\n");
        // @TODO: Crash here...
    }
}
