#include "../sleep.h"
#include "../lox-internal.h"

static void __fake_sleep(ulong ms) {
    unused(ms);
}

void sleep(ulong ms) {
    if (lox_sleep_provider != NULL) {
        lox_sleep_provider(ms);
    } else {
        __fake_sleep(ms);
    }
}
