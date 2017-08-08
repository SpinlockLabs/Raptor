#include "console.h"

#include <kernel/pm/pm.h>

static void debug_poweroff(tty_t* tty, const char* input) {
    unused(tty);
    unused(input);

    pm_power_off();
}

void debug_power_init(void) {
    debug_register_command("poweroff", debug_poweroff);
}
