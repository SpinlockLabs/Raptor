#include "../lox-internal.h"
#include "../memory.h"
#include "../string.h"

#ifndef _MSC_VER
__attribute__((malloc))
#endif
void* malloc(size_t size) {
    if (lox_allocate_provider != NULL) {
        return lox_allocate_provider(size);
    }
    return NULL;
}

#ifndef _MSC_VER
__attribute__((malloc))
#endif
void* zalloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr != NULL) {
        memset(ptr, 0, size);
    }
    return ptr;
}

#ifndef _MSC_VER
__attribute__((malloc))
#endif
void* calloc(size_t count, size_t size) {
    return zalloc(count * size);
}

#ifndef _MSC_VER
__attribute__((malloc))
#endif
void* realloc(void* ptr, size_t size) {
    if (lox_reallocate_provider != NULL) {
        return lox_reallocate_provider(ptr, size);
    }
    void* ret = malloc(size);
    memcpy(ret, ptr, size);
    free(ptr);
    return ret;
}

void free(void *ptr) {
    if (lox_free_provider != NULL) {
        lox_free_provider(ptr);
    }
}
