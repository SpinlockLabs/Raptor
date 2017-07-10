#include "lox-internal.h"
#include "memory.h"
#include "string.h"

__attribute__((malloc)) void* malloc(size_t size) {
    if (lox_allocate_provider != NULL) {
        return lox_allocate_provider(size);
    }
    return NULL;
}

__attribute__((malloc)) void* zalloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr != NULL) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void free(void *ptr) {
    if (lox_free_provider != NULL) {
        lox_free_provider(ptr);
    }
}
