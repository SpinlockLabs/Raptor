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

    if ((c & 128) == 128) {
        vga_writestring("released\n");
    }

    char kt = key_types[c];
    switch (kt) {
        case N:
            vga_putchar((shift ? kb_usu[c] : kb_usl[c]));
            break;
        case S:
            switch (c) {
                case 0x2A:
                case 0x36:
                    shift = true;
                    break;
                case 0xAA:
                case 0xB6:
                    shift = false;
                    break;
            }
            break;
        case F:
            vga_writestring("F\n");
            break;
    }

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

