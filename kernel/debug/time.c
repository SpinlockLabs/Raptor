#include "console.h"

#include <kernel/time.h>

static void debug_time(tty_t* tty, const char* input) {
    unused(input);

    time_t* time = malloc(sizeof(time_t));
    time_get(time);

    tty_printf(tty, "%d-%d-%d %d:%d:%d\n",
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
