#include "keyboard.h"
#include "irq.h"
#include "io.h"
#include "vga.h"
#include "x86.h"
#include <string.h>
#include <stdbool.h>

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

    char kt = key_types[c];
    switch (kt) {
        case N:
            vga_writestring("N\n");
            break;
        case S:
            vga_writestring("S\n");
            break;
        case F:
            vga_writestring("F\n");
            break;
    }

    return 1;
}

void keyboard_init(void) {
    irq_add_handler(IRQ1, &keyboard_callback);
}

