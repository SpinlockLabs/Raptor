#include <liblox/lox-internal.h>

#define __USE_GNU
#include <dlfcn.h>
#undef __USE_GNU

void __output_string(char* msg) {
    void (*libc_printf)(char*, ...) = dlsym(RTLD_NEXT, "printf");
    libc_printf("%s", msg);
}

void __output_char(char c) {
    void (*libc_putc)(char) = dlsym(RTLD_NEXT, "putchar");
    libc_putc(c);
}

void* __malloc(size_t size) {
    void* (*libc_malloc)(size_t) = dlsym(RTLD_NEXT, "malloc");
    return libc_malloc(size);
}

void* __realloc(void* ptr, size_t size) {
    void* (*libc_realloc)(void*, size_t) = dlsym(RTLD_NEXT, "realloc");
    return libc_realloc(ptr, size);
}

void* __valloc(size_t size) {
    void* (*libc_valloc)(size_t) = dlsym(RTLD_NEXT, "valloc");
    return libc_valloc(size);
}

void __free(void* ptr) {
    void (*libc_free)(void*) = dlsym(RTLD_NEXT, "free");
    libc_free(ptr);
}

void __sleep(ulong ms) {
    unused(ms);
}

void __abort(char* msg) {
    void (*libc_abort)(char*) = dlsym(RTLD_NEXT, "abort");
    libc_abort(msg);
}

void (*lox_output_string_provider)(char*) = __output_string;
void (*lox_output_char_provider)(char) = __output_char;
void* (*lox_allocate_provider)(size_t) = __malloc;
void* (*lox_aligned_allocate_provider)(size_t) = __valloc;
void (*lox_free_provider)(void*) = __free;
void* (*lox_reallocate_provider)(void*, size_t) = __realloc;
void (*lox_sleep_provider)(ulong) = __sleep;
void (*lox_abort_provider)(char*) = __abort;
