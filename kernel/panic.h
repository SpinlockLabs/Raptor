#include <liblox/io.h>

#define panic(msg) puts("PANIC! " msg "\n"); while (1) { asm("hlt"); }
