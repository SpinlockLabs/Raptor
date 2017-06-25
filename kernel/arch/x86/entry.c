#include <kcommon.h>
#include <io.h>

#include "gdt.h"
#include "irq.h"
#include "pm.h"
#include "vga.h"

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

    puts(DEBUG "Entering protected mode. See you on the other side!\n");
    enter_protected_mode();
}

used noreturn void kernel_protected_main(void) {
    puts(DEBUG "Hello from protected mode!\n");

    while (1) {
        asm ("nop");
    }
}
