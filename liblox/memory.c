#include "memory.h"
#include "lox-internal.h"

void* malloc(size_t size) {
    if (lox_allocate_provider != NULL) {
        return lox_allocate_provider(size);
    }
    return NULL;
}

void free(void *ptr) {
    if (lox_free_provider != NULL) {
        lox_free_provider(ptr);
    }
}
