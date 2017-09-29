#include "console.h"

#include <liblox/memory.h>

#include <kernel/time.h>
#include <kernel/timer.h>

static void debug_time(tty_t* tty, const char* input) {
    unused(input);

    rtime_t* time = zalloc(sizeof(rtime_t));
    time_get(time);

    tty_printf(tty, "%2d-%2d-%4d %2d:%2d:%2d\n",
               time->month,
               time->day,
               time->year,
               time->hour,
               time->minute,
               time->second);

    free(time);
}

static void debug_ticks(tty_t* tty, const char* input) {
    unused(input);

    ulong ticks = timer_get_ticks();
    tty_printf(tty, "%d\n", ticks);
}

void debug_time_init(void) {
    debug_register_command((console_command_t) {
        .name = "time",
        .group = "time",
        .help = "Show the current system time",
        .cmd = debug_time
    });
    debug_register_command((console_command_t) {
        .name = "ticks",
        .group = "time",
        .help = "Show the current system ticks",
        .cmd = debug_ticks
    });
}
