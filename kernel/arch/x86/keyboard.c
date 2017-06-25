#include <liblox/common.h>

#include "io.h"
#include "irq.h"
#include "keyboard.h"
#include "vga.h"

/**
 * Callback for keyboard interrupt(IRQ 1).
 */
static int keyboard_callback(regs_t *regs) {
    unused(regs);

    char c = 0;
    bool up;

    c = inb(0x60);
    up = (bool) (c & 0x80);
    if (up) {
        c ^= 0x80;
    }

    unsigned int idx = (unsigned int) c;
    char kt = key_types[idx];
    switch (kt) {
        case N:
            if (!up) {
                vga_putchar((shift ? kb_usu[idx] : kb_usl[idx]));
            }
            break;
        case S:
            switch (idx) {
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
