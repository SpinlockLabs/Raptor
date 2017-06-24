#include <kcommon.h>
#include <lox-internal.h>
#include <io.h>

#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "timer.h"
#include "vga.h"
#include "keyboard.h"

void lox_output_string_vga(char* msg) {
    vga_writestring(msg);
}

void (*lox_output_string_provider)(char*) = lox_output_string_vga;

used void kernel_main(void) {
    vga_init();

    puts("Raptor kernel\n");

    gdt_init();
    puts("GDT Initialized\n");
    idt_init();
    puts("IDT Initialized\n");
    isr_init();
    puts("ISRs Initialized\n");
    irq_init();
    puts("IRQs Initialized\n");
    timer_init(50);
    puts("PIT Initialized\n");
    keyboard_init();
    puts("Keyboard Initialized\n");

    puts("Entering idle state\n");

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    // Let's idle, while continuously enabling interrupts.
    for (;;) {
        int_enable();
        asm("hlt");
    }
#pragma clang diagnostic pop
}
