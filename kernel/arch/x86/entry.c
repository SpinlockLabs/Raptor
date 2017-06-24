#include "entry.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "timer.h"
#include "vga.h"

void kernel_main(void) {
    vga_init();
    vga_writestring("Raptor kernel\n");

    gdt_init();
    vga_writestring("GDT Initialized\n");
    idt_init();
    vga_writestring("IDT Initialized\n");
    isr_init();
    vga_writestring("ISRs Initialized\n");
    irq_init();
    vga_writestring("IRQs Initialized\n");
    timer_init(50);
    vga_writestring("PIT Initialized\n");
    keyboard_init();
    vga_writestring("Keyboard Initialized\n");

    vga_writestring("Entering idle state\n");
    // Let's idle, while continuously enabling interrupts.
    for (;;) {
        int_enable();
        asm("hlt");
    }
}

