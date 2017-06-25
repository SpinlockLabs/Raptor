#include <liblox/common.h>
#include <liblox/io.h>

#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "keyboard.h"
#include "timer.h"
#include "userspace.h"
#include "vga.h"

const uint32_t kProcessorIdIntel = 0x756e6547;
const uint32_t kProcessorIdAMD = 0x68747541;

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

    uint32_t ebx = 0;
    get_cpuid(0, 0, &ebx, 0, 0);
    if (ebx == kProcessorIdIntel) {
        puts(INFO "Processor Type: Intel\n");
    } else if (ebx == kProcessorIdAMD) {
        puts(INFO "Processor Type: AMD\n");
    } else {
        puts(INFO "Processor Type: Unknown\n");
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

    puts(DEBUG "Entering idle state\n");

    userspace_jump(NULL, 0xB0000000);

    /*for (;;) {
        int_enable();
        asm("hlt");
    }*/
}
