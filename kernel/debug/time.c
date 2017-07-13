#include "console.h"

#include <kernel/time.h>

static void debug_time(tty_t* tty, const char* input) {
    unused(input);

    time_t* time = zalloc(sizeof(time_t));
    time_get(time);

    tty_printf(tty, "%2d-%2d-%4d %2d:%2d:%2d\n",
               time->month,
               time->day,
               time->year,
               time->hour,
               time->minute,
               time->second);
}

void debug_time_init(void) {
    debug_console_register_command("time", debug_time);
}
