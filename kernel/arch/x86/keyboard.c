#include "keyboard.h"
#include "irq.h"
#include "io.h"
#include "vga.h"

/**
 * Callback for keyboard interrupt(IRQ 1).
 */
static int keyboard_callback(regs_t *regs) {
    char c = 0;
    bool up;

    c = inb(0x60);
    up = (bool) (c & 0x80);
    if (up) {
        c ^= 0x80;
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
                default: break;
            }
            break;
        default: break;
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
