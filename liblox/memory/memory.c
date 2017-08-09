#include "../lox-internal.h"
#include "../memory.h"
#include "../string.h"

__attribute__((malloc))
void* malloc(size_t size) {
    return lox_allocate_provider(size);
}

__attribute__((malloc))
void* zalloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr != NULL) {
        memset(ptr, 0, size);
    }
    return ptr;
}

__attribute__((malloc))
void* calloc(size_t count, size_t size) {
    return zalloc(count * size);
}

__attribute__((malloc))
void* realloc(void* ptr, size_t size) {
    return lox_reallocate_provider(ptr, size);
}

void free(void *ptr) {
    lox_free_provider(ptr);
}
