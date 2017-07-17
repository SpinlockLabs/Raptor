#include "console.h"

#include <kernel/cmdline.h>

static void show_cmdline(tty_t* tty, const char* input) {
    unused(input);

    tty_printf(tty, "Command Line: %s\n", cmdline_get());
}

static void has_flag(tty_t* tty, const char* input) {
    if (cmdline_bool_flag((char*) input)) {
        tty_printf(tty, "Flag '%s' exists.\n", input);
    } else {
        tty_printf(tty, "Flag '%s' does not exist.\n", input);
    }
}

static void debug_read(tty_t* tty, const char* input) {
    char* value = cmdline_read((char*) input);

    if (value != NULL) {
        tty_printf(tty, "%s\n", value);
    } else {
        tty_printf(tty, "Parameter not found.\n");
    }
}

void debug_cmdline_init(void) {
    debug_console_register_command("cmdline", show_cmdline);
    debug_console_register_command("cmdline-has", has_flag);
    debug_console_register_command("cmdline-read", debug_read);
}
