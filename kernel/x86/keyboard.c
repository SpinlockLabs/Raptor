#include "keyboard.h"
#include "irq.h"
#include "io.h"
#include "vga.h"
#include "x86.h"
#include <string.h>

/**
 * Callback for keyboard interrupt(IRQ 1).
 */
static int keyboard_callback(regs_t *regs) {
    char c = 0;

    do {
        if (inb(0x60) != c) {
            c = inb(0x60);
            if (c > 0) {
                break;
            }
        }
    } while(1);

    char* cs;
    itoa(c, cs, 16);

    vga_writestring("Code ");
    vga_writestring(cs);
    vga_writestring("\n");

    return 1;
}

void keyboard_init(void) {
    irq_add_handler(IRQ1, &keyboard_callback);
}

