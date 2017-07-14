#include "commands.h"
#include "console.h"

#define GROUP(n) \
    extern void debug_## n ##_init(); \
    debug_## n ##_init()

#define CMD(f, n) \
    extern void debug_##f(tty_t*, const char*); \
    debug_console_register_command(n, debug_##f)

void debug_init_commands(void) {
    GROUP(network);
    GROUP(kheap);
    GROUP(time);
    GROUP(cmdline);
    GROUP(disk);
}
