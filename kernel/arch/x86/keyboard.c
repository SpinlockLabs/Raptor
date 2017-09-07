#include <liblox/common.h>
#include <kernel/input/input.h>
#include <kernel/input/events.h>

#include "io.h"
#include "irq.h"
#include "keyboard.h"
#include "vga.h"

#define N 0x01 // Normal
#define S 0x02 // Special
#define F 0x03 // Function

static input_device_t* kbd;

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

    if (kbd != NULL && kbd->events != NULL) {
        input_event_key_state_t event = {
            .event.type = up ? INPUT_EVENT_TYPE_KEY_UP : INPUT_EVENT_TYPE_KEY_DOWN,
            .event.data_size = sizeof(input_event_key_state_t) - sizeof(input_event_t),
            .key = idx
        };
        epipe_deliver(kbd->events, &event);
    }

    switch (kt) {
        case N:
            if (!up) {
                uint8_t cc = (uint8_t) (shift ? kb_usu[idx] : kb_usl[idx]);

                if (vga_pty != NULL) {
                    tty_read_event_t event = {
                        .data = &cc,
                        .size = 1,
                        .tty = vga_pty
                    };
                    epipe_deliver(&vga_pty->reads, &event);
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
        default:
            break;
    }

    if (!up && vga_pty != NULL) {

        if (c == 0x4b) {
            tty_read_event_t event = {
                .data = (uint8_t*) "\x1b[D",
                .size = 3,
                .tty = vga_pty
            };
            epipe_deliver(&vga_pty->reads, &event);
        } else if (c == 0x4d) {
            tty_read_event_t event = {
                .data = (uint8_t*)  "\x1b[C",
                .size = 3,
                .tty = vga_pty
            };
            epipe_deliver(&vga_pty->reads, &event);
        }
    }

    return 1;
}

void keyboard_init(void) {
    kbd = input_device_create("ps2-keyboard", INPUT_DEV_CLASS_KEYBOARD);
    input_device_register(device_root(), kbd);

    irq_add_handler(IRQ1, &keyboard_callback);
}
