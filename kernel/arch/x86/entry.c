#include <kcommon.h>
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

void lox_output_char_vga(char c) {
    vga_putchar(c);
}

void (*lox_output_string_provider)(char*) = lox_output_string_vga;
void (*lox_output_char_provider)(char) = lox_output_char_vga;

used void kernel_main(void) {
    vga_init();

    puts(INFO "Raptor kernel\n");

    uint32_t ebx;
    get_cpuid(0, 0, &ebx, 0, 0);
    if (ebx == 0x756e6547) {
        puts(INFO "Processor Type: Intel\n");
    } else if (ebx == 0x68747541) {
        puts(INFO "Processor Type: AMD\n");
    }

    gdt_init();
    puts(DEBUG "GDT Initialized\n");
    idt_init();
    puts(DEBUG "IDT Initialized\n");
    isr_init();
    puts(DEBUG "ISRs Initialized\n");
    irq_init();
    puts(DEBUG "IRQs Initialized\n");
    timer_init(50);
    puts(DEBUG "PIT Initialized\n");
    keyboard_init();
    puts(DEBUG "Keyboard Initialized\n");

    puts("Entering idle state\n");

    for (;;) {
        int_enable();
        asm("hlt");
    }
}
