#include <liblox/common.h>
#include <liblox/string.h>
#include <liblox/io.h>

#include <kernel/entry.h>
#include <kernel/panic.h>
#include <kernel/cmdline.h>
#include <kernel/timer.h>

#include "cmdline.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "pci_init.h"
#include "userspace.h"
#include "io.h"
#include "vga.h"

const uint32_t kProcessorIdIntel = 0x756e6547;
const uint32_t kProcessorIdAMD = 0x68747541;

void lox_output_char_ebl(char c) {
    outb(0x3F8, c);
}

void lox_output_string_ebl(char* msg) {
    if (msg == NULL) {
        return;
    }

    size_t len = strlen(msg);
    for (size_t i = 0; i < len; ++i) {
        lox_output_char_ebl(msg[i]);
    }
}

void lox_output_string_vga(char* msg) {
    if (msg == NULL) {
        return;
    }
    vga_writestring(msg);
    lox_output_string_ebl(msg);
}

void lox_output_char_vga(char c) {
    vga_putchar(c);
    lox_output_char_ebl(c);
}

used void arch_panic_handler(nullable char *msg) {
    asm("cli;");

    if (msg != NULL) {
        vga_putchar('\n');
        vga_writestring("[PANIC] ");
        vga_writestring(msg);
        vga_putchar('\n');
    }

    while (1) {
        asm("hlt;");
    }
}

void (*lox_output_string_provider)(char*) = lox_output_string_ebl;
void (*lox_output_char_provider)(char) = lox_output_char_ebl;

used void kernel_main(multiboot_t *_mboot, uint32_t mboot_hdr) {
    if (mboot_hdr != MULTIBOOT_EAX_MAGIC) {
        return;
    }

    mboot = _mboot;

    init_cmdline(mboot);

    vga_init();

    if (!cmdline_bool_flag("disable-vga")) {
        lox_output_string_provider = lox_output_string_vga;
        lox_output_char_provider = lox_output_char_vga;
    }

    if (cmdline_bool_flag("debug")) {
        puts(DEBUG "cmdline: ");
        puts(get_cmdline());
        putc('\n');
    }

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
    puts(DEBUG "GDT initialized.\n");
    idt_init();
    puts(DEBUG "IDT initialized.\n");
    isr_init();
    puts(DEBUG "ISRs initialized.\n");
    irq_init();
    puts(DEBUG "IRQs initialized.\n");
    timer_init(1000);
    puts(DEBUG "PIT initialized.\n");

    breakpoint("pci-init");
    puts(DEBUG "Probing PCI devices...\n");
    pci_init();
    puts(DEBUG "PCI probe done.\n");

    if (cmdline_bool_flag("enable-userspace-jump")) {
        breakpoint("userspace-jump");
        puts(DEBUG "Jumping to userspace...\n");
        userspace_jump(NULL, 0xB0000000);
    }

    kernel_init();
}
