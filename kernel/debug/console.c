#include "console.h"

#include <liblox/string.h>
#include <liblox/memory.h>
#include <liblox/strbuf/strbuf.h>
#include <kernel/arch/x86/process.h>

#include "commands.h"

#define CONSOLE_BUFFER_SIZE 1024
#define CONSOLE_CMD_MAX_SIZE 64

static hashmap_t* console_commands;

typedef struct debug_console {
    strbuf_t buffer;
} debug_console_t;

void debug_console_trigger(tty_t* tty, char* str) {
    char cmd[CONSOLE_CMD_MAX_SIZE];
    memset(cmd, 0, CONSOLE_CMD_MAX_SIZE);
    char args[CONSOLE_BUFFER_SIZE];
    memset(args, 0, CONSOLE_BUFFER_SIZE);

    while (*str == ' ') {
        str++;
    }

    size_t index = 0;
    while (*str != ' ' && index <= (CONSOLE_CMD_MAX_SIZE - 1)) {
        cmd[index++] = *(str++);
    }

    if (index == 0) {
        return;
    }

    while (*str == ' ') {
        str++;
    }

    index = 0;
    while (*str != '\0') {
        args[index++] = *(str++);
    }

    if (*cmd == '\0') {
        return;
    }

    if (!hashmap_has(console_commands, cmd)) {
        tty_printf(tty, "Unknown Command: %s\n", cmd);
        return;
    }

    debug_console_command_t handle = ((console_command_t*) hashmap_get(console_commands, cmd))->cmd;
    handle(tty, args);
}

void debug_console_command_reset(tty_t* tty) {
    debug_console_t* console = tty->internal.controller;
    strbuf_clear(&console->buffer);
    tty->status.execute_post_write = true;
}

static void debug_console_handle_data(epipe_t* pipe, void* event, void* extra) {
    unused(pipe);
    unused(extra);

    tty_read_event_t* read = event;
    tty_t* tty = read->tty;

    if (read->size == 0) {
        return;
    }

    debug_console_t* console = read->tty->internal.controller;

    if (read->size == 1 && (read->data[0] == '\b' || read->data[0] == 0x7f)) {
        if (strbuf_backspace(&console->buffer)) {
            if (tty->flags.echo) {
                tty_write_string(tty, "\b");
            }
        }

        return;
    }

    if (read->size >= 3 && read->data[0] == '\x1b' && read->data[1] == '[') {
        char c = read->data[2];
        if (c == 'D') {
            if (strbuf_move_left(&console->buffer)) {
                if (tty->flags.echo) {
                    tty_write_string(tty, "\x1b[D");
                }
            }
            return;
        }

        if (c == 'C') {
            if (strbuf_move_right(&console->buffer)) {
                if (tty->flags.echo) {
                    tty_write_string(tty, "\x1b[C");
                }
            }
            return;
        }
    }

    bool triggered = false;

    for (uint i = 0; i < read->size; i++) {
        char c = read->data[i];

        tty->status.execute_post_write = false;
        if (tty->flags.echo) {
            uint8_t echo_buffer[1] = {0};
            echo_buffer[0] = (uint8_t) c;
            tty_write(tty, echo_buffer, 1);
        }

        if (c == '\n' || c == '\r') {
            triggered = true;
            char* cmd = strbuf_read(&console->buffer);
            debug_console_trigger(tty, cmd);
            debug_console_command_reset(tty);
        } else {
            if (!strbuf_putc(&console->buffer, c)) {
                tty_printf(tty, "Command buffer full. Resetting.\n");
                strbuf_clear(&console->buffer);
                return;
            }
            tty->status.execute_post_write = true;
        }
    }

    if (triggered) {
        tty_write_string(tty, "> ");
    }
}

static void debug_console_post_write(tty_t* tty, const uint8_t* buffer, size_t size) {
    unused(tty);

    if (size == 0) {
        return;
    }

    if (buffer[size - 1] == '\n') {
        tty_write_string(tty, "> ");
    }
}

void debug_console_init(void) {
    console_commands = hashmap_create(10);
}

static void debug_help(tty_t* tty, const char* input) {
    unused(input);

    list_t* values = hashmap_values(console_commands);
    console_command_t* curVal;
    list_for_each(node, values) {
        curVal = (console_command_t*)node->value;
        if ((strlen(input) == 0) ||
            (strlen(input) > 0 && strcmp(input, curVal->group) == 0)) {
            tty_printf(tty, "- %s: %s\n", curVal->name, curVal->help);
        }
    }
    list_free(values);
}

static void debug_crash(tty_t* tty, const char* input) {
    unused(input);
    unused(tty);

    memcpy(NULL, NULL, 1);
}

void debug_console_start(void) {
    {
        debug_register_command((console_command_t) {
            .name = "help",
            .group = "debug",
            .help = "Show this help menu",
            .cmd = debug_help
        });
        debug_register_command((console_command_t) {
            .name = "crash",
            .group = "debug",
            .help = "Crash the system",
            .cmd = debug_crash
        });

        debug_init_commands();
    }

    list_t* ttys = tty_get_all();
    list_for_each(node, ttys) {
        tty_t* tty = node->value;
        if (tty->flags.allow_debug_console) {
            debug_console_t* console = zalloc(sizeof(debug_console_t) + CONSOLE_BUFFER_SIZE + 1);
            tty->internal.controller = console;
            strbuf_init(&console->buffer, CONSOLE_BUFFER_SIZE);
            epipe_add_handler(&tty->reads, debug_console_handle_data, NULL);
            tty->ops.post_write = debug_console_post_write;
            tty_write_string(tty, "[[Raptor Debug Console Started]]\n> ");
        }
    }
    list_free(ttys);
}

void debug_register_command(console_command_t cmd) {
    if (console_commands == NULL) {
        return;
    }

    console_command_t* cc_ptr = malloc(sizeof(console_command_t));
    memcpy(cc_ptr, &cmd, sizeof(console_command_t));

    hashmap_set(console_commands, cmd.name, cc_ptr);
}
