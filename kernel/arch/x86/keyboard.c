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
                uint8_t cc = (uint8_t) (shift ? kb_usu[idx] : kb_usl[idx]);
                if (vga_pty != NULL && vga_pty->handle_read != NULL) {
                    vga_pty->handle_read(vga_pty, &cc, 1);
                }
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

    return 1;
}

void keyboard_init(void) {
    irq_add_handler(IRQ1, &keyboard_callback);
}
