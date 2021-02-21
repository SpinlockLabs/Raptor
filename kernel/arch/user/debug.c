#include "debug.h"
#include "env.h"

#include <kernel/debug/console.h>

static void debug_exit(tty_t* tty, const char* input) {
    unused(tty);
    unused(input);

    raptor_user_exit();
}

void debug_user_init(void) {
    debug_register_command((console_command_t) {
        .name = "exit",
        .group = "user",
        .help = "Exit the kernel in userland process",
        .handler = debug_exit
    });
}
