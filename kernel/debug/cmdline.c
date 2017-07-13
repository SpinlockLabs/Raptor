#include "console.h"

#include <kernel/tty.h>
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

void debug_cmdline_init(void) {
    debug_console_register_command("cmdline", show_cmdline);
    debug_console_register_command("cmdline-has", has_flag);
}
