#pragma once

#include <kernel/tty/tty.h>

typedef void (*debug_console_command_t)(tty_t* tty, const char* input);

typedef struct console_command {
    char* name;
    char* group;
    char* help;
    debug_console_command_t cmd;
} console_command_t;

void debug_register_command(console_command_t con_cmd);
void debug_console_init(void);
void debug_console_start(void);
