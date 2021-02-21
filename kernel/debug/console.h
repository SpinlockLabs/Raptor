#pragma once

#include <kernel/tty/tty.h>

typedef void (*debug_console_command_t)(tty_t* tty, const char* input);

typedef struct console_command {
    char* name;
    char* group;
    char* help;
    debug_console_command_t handler;
} console_command_t;

void debug_register_command(console_command_t cmd);
void debug_console_init(void);
void debug_console_start(void);
void debug_console_trigger(tty_t* tty, char* str);
void debug_console_command_reset(tty_t* tty);
