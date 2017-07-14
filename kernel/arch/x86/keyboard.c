#include <liblox/common.h>

#include "io.h"
#include "irq.h"
#include "keyboard.h"
#include "vga.h"

#define N 0x01 // Normal
#define S 0x02 // Special
#define F 0x03 // Function

static bool shift = false;

static char key_types[] = {
    0, S, N, N, N, N, N, N,
    N, N, N, N, N, N, N, N,
    N, N, N, N, N, N, N, N,
    N, N, N, N, N, S, N, N,
    N, N, N, N, N, N, N, N,
    N, N, S, N, N, N, N, N,
    N, N, N, N, N, N, S, N,
    S, N, S, F, F, F, F, F,
    F, F, F, F, F, S, S, S,
    S, S, S, S, S, S, S, S,
    S, S, S, S, S, S, S, F,
    F, S, S, S, S, S, S, S,
    S, S, S, S, S, S, S, S,
    S, S, S, S, S, S, S, S,
    S, S, S, S, S, S, S, S,
    S, S, S, S, S, S, S, S
};

static char kb_usl[128] = {
    0, 27, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, '-', 0, 0, 0, '+', 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

static char kb_usu[128] = {
    0, 27, '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{', '}', '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, '-', 0, 0, 0, '+', 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/**
 * Callback for keyboard interrupt(IRQ 1).
 */
static int keyboard_callback(cpu_registers_t* regs) {
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

                vga_putchar(cc);

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
