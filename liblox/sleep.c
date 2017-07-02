#include "sleep.h"
#include "lox-internal.h"

void sleep(ulong ms) {
    if (lox_sleep_provider != NULL) {
        lox_sleep_provider(ms);
    }
}
