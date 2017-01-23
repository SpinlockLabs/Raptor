#include "entry.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "timer.h"
#include "tty.h"

void kernel_main(void) {
    terminal_init();
    terminal_writestring("Raptor kernel\n");

    gdt_init();
    terminal_writestring("GDT Initialized\n");
    idt_init();
    terminal_writestring("IDT Initialized\n");
    isr_init();
    terminal_writestring("ISRs Initialized\n");
    irq_init();
    terminal_writestring("IRQs Initialized\n");
    timer_init(50);
    terminal_writestring("PIT Initialized\n");

    terminal_writestring("Entering idle state\n");
    // Let's idle, while continuously enabling interrupts.
    for (;;) {
        int_enable();
        asm("hlt");
    }
}

