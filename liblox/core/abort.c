#include "../abort.h"
#include "../lox-internal.h"

void abort(char* msg) {
    if (lox_abort_provider != NULL) {
        lox_abort_provider(msg);
    }

    while (1) {}
}
