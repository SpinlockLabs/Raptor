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
    bool up = false;

    c = inb(0x60);
    up = c & 0x80;
    if (up) {
        c = c ^= 0x80;
    }

    char kt = key_types[c];
    switch (kt) {
        case N:
            if (!up) {
                vga_putchar((shift ? kb_usu[c] : kb_usl[c]));
            }
            break;
        case S:
            switch (c) {
                case 0x2a:
                case 0x36:
                    shift = !up;
                    break;
            }
            break;
    }

    /*char* cs;
    itoa(c, cs, 16);
    vga_writestring("Code ");
    vga_writestring(cs);
    vga_writestring("\n");*/

    return 1;
}

void keyboard_init(void) {
    irq_add_handler(IRQ1, &keyboard_callback);
}

