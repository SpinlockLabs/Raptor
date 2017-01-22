#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "entry.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "tty.h"

void kernel_main(void) {
    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    terminal_init();

    asm volatile("int $0x0");

    terminal_writestring("Hello World!\n");
    terminal_writestring("Hello World!\n");

    // Let's idle, while continuously enabling interrupts.
    for (;;) {
        int_enable();
        asm("hlt");
    }
}

