#pragma once

#include <kernel/tty.h>

typedef void (*debug_console_command_t)(tty_t* tty, const char* input);

void debug_register_command(char* name, debug_console_command_t cmd);
void debug_console_init(void);
void debug_console_start(void);
