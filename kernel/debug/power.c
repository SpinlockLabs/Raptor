#include "console.h"

#include <kernel/pm/pm.h>

static void debug_poweroff(tty_t* tty, const char* input) {
    unused(tty);
    unused(input);

    pm_power_off();
}

static void debug_reboot(tty_t* tty, const char* input) {
    unused(tty);
    unused(input);
    pm_reboot();
}

void debug_power_init(void) {
    debug_register_command("poweroff", debug_poweroff);
    debug_register_command("reboot", debug_reboot);
}
