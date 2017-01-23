#include "keyboard.h"
#include "irq.h"
#include "io.h"
#include "vga.h"
#include "x86.h"

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

    vga_writestring("KB Event\n");
}

void keyboard_init(void) {
    irq_add_handler(IRQ1, &keyboard_callback);
}

