#include "../string.h"
#include "../memory.h"
#include "../common.h"

char* strdup(const char* s) {
    size_t l = strlen(s);
    char* d = malloc(l + 1);
    ensure_allocated(d);
    return memcpy(d, s, l + 1);
}
