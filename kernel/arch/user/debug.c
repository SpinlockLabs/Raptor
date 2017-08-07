#include "debug.h"
#include "env.h"

#include <kernel/debug/console.h>

static void debug_exit(tty_t* tty, const char* input) {
    unused(tty);

    raptor_user_exit();
}

void debug_user_init(void) {
    debug_register_command("exit", debug_exit);
}
