#include "isr.h"
#include "vga.h"

#define isr(i) idt_set_gate(i, (uint32_t)_isr##i, 0x08, 0x8E)

void isr_add_handler(size_t isr, irq_handler_t handler) {
    isr_routines[isr] = handler;
}

void isr_rem_handler(size_t isr) {
    isr_routines[isr] = 0;
}

void isr_init(void) {
    isr(0);
    isr(1);
    isr(2);
    isr(3);
    isr(4);
    isr(5);
    isr(6);
    isr(7);
    isr(8);
    isr(9);
    isr(10);
    isr(11);
    isr(12);
    isr(13);
    isr(14);
    isr(15);
    isr(16);
    isr(17);
    isr(18);
    isr(19);
    isr(20);
    isr(21);
    isr(22);
    isr(23);
    isr(24);
    isr(25);
    isr(26);
    isr(27);
    isr(28);
    isr(29);
    isr(30);
    isr(31);
    isr(127);
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
