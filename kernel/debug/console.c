#include "console.h"

#include <liblox/string.h>
#include <liblox/memory.h>
#include <liblox/strbuf/strbuf.h>

#include "commands.h"

#define CONSOLE_BUFFER_SIZE 1024
#define CONSOLE_CMD_MAX_SIZE 64

static hashmap_t* console_commands;

typedef struct debug_console {
    strbuf_t buffer;
} debug_console_t;

static void debug_console_trigger(tty_t* tty, char* str) {
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

    debug_console_command_t handle = hashmap_get(console_commands, cmd);
    handle(tty, args);
}

static void debug_console_handle_data(tty_t* tty, const uint8_t* buffer, size_t size) {
    unused(tty);

    if (size == 0) {
        return;
    }

    debug_console_t* console = tty->internal.controller;

    if (size == 1 && (buffer[0] == '\b' || buffer[0] == 0x7f)) {
        if (strbuf_backspace(&console->buffer)) {
            if (tty->flags.echo) {
                tty_write_string(tty, "\b");
            }
        }

        return;
    }

    if (size >= 3 && buffer[0] == '\x1b' && buffer[1] == '[') {
        char c = buffer[2];
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

    for (uint i = 0; i < size; i++) {
        char c = buffer[i];

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
            strbuf_clear(&console->buffer);
        } else {
            if (!strbuf_putc(&console->buffer, c)) {
                tty_printf(tty, "Command buffer full. Resetting.\n");
                strbuf_clear(&console->buffer);
                return;
            }
        }
        tty->status.execute_post_write = true;
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

    list_t* keys = hashmap_keys(console_commands);
    list_for_each(node, keys) {
        tty_printf(tty, "- %s\n", node->value);
    }
    list_free(keys);
}

static void debug_crash(tty_t* tty, const char* input) {
    unused(input);
    unused(tty);

    memcpy(NULL, NULL, 1);
}

void debug_console_start(void) {
    {
        debug_register_command("help", debug_help);
        debug_register_command("crash", debug_crash);

        debug_init_commands();
    }

    list_t* ttys = tty_get_all();
    list_for_each(node, ttys) {
        tty_t* tty = node->value;
        if (tty->flags.allow_debug_console) {
            debug_console_t* console = zalloc(sizeof(debug_console_t) + CONSOLE_BUFFER_SIZE + 1);
            tty->internal.controller = console;
            strbuf_init(&console->buffer, CONSOLE_BUFFER_SIZE);
            tty->handle_read = debug_console_handle_data;
            tty->post_write = debug_console_post_write;
            tty_write_string(tty, "[[Raptor Debug Console Started]]\n> ");
        }
    }
    list_free(ttys);
}

void debug_register_command(char* name, debug_console_command_t cmd) {
    if (console_commands == NULL) {
        return;
    }

    hashmap_set(console_commands, name, cmd);
}
