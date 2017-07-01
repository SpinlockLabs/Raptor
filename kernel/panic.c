#include "panic.h"

// Architecture-specific panic handler.
extern void (*arch_panic_handler)(nullable char* msg);

noreturn void panic(nullable char *msg) {
    if (arch_panic_handler != NULL) {
        arch_panic_handler(msg);
    }
    while (1) {}
}
