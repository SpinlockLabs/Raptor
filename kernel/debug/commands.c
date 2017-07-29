#include "commands.h"

#define GROUP(n) \
    extern void debug_## n ##_init(void); \
    debug_## n ##_init()

void debug_init_commands(void) {
    GROUP(network);
    GROUP(kheap);
    GROUP(time);
    GROUP(cmdline);
    GROUP(disk);
    GROUP(fs);
}
