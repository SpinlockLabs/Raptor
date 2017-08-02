#include <liblox/abort.h>
#include <stdint.h>
#include "../string.h"

int strcmp(const char* l, const char* r) {
    if ((uintptr_t) l < 0x1000 || (uintptr_t) r < 0x1000) {
        abort("Bad string compare.");
    }

    for (; *l == *r && *l; l++, r++) {}
    return *(unsigned char*) l - *(unsigned char*) r;
}
