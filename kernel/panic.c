#include "panic.h"

// Architecture-specific panic handler.
extern void arch_panic_handler(nullable char* msg);

does_not_return void panic(nullable char *msg) {
    arch_panic_handler(msg);
    while (1) {}
}

void (*lox_abort_provider)(char* msg) = panic;
