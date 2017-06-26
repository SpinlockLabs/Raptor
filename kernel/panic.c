#include "panic.h"

noreturn void panic(nullable char *msg) {
    if (arch_panic_handler != NULL) {
        arch_panic_handler(msg);
    }
    while (1) {}
}
