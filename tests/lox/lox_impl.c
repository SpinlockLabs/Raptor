#include <liblox/lox-internal.h>

#ifndef __WIN32__

#ifndef __CYGWIN__
#define __USE_GNU
#include <dlfcn.h>
#undef __USE_GNU

void* libc_sym(char* name) {
    return dlsym(RTLD_NEXT, name);
}
#else
#include <dlfcn.h>
static void* libc = NULL;

void* libc_sym(char* name) {
    if (libc == NULL) {
        libc = dlopen("cygwin1.dll", 0);
    }
    return dlsym(libc, name);
}
#endif

#else
#include <windows.h>

static HMODULE libc = NULL;

void* libc_sym(char* name) {
    if (libc == NULL) {
        libc = LoadLibraryA("msvcrt.dll");
    }
    return GetProcAddress(libc, sym);
}
#endif

void __output_string(char* msg) {
    void (*libc_printf)(char*, ...) = libc_sym("printf");
    libc_printf("%s", msg);
}

void __output_char(char c) {
    void (*libc_putc)(char) = libc_sym("putchar");
    libc_putc(c);
}

void* __malloc(size_t size) {
    void* (*libc_malloc)(size_t) = libc_sym("malloc");
    return libc_malloc(size);
}

void* __realloc(void* ptr, size_t size) {
    void* (*libc_realloc)(void*, size_t) = libc_sym("realloc");
    return libc_realloc(ptr, size);
}

void* __valloc(size_t size) {
    void* (*libc_valloc)(size_t) = libc_sym("valloc");
    return libc_valloc(size);
}

void __free(void* ptr) {
    void (*libc_free)(void*) = libc_sym("free");
    libc_free(ptr);
}

void __sleep(ulong ms) {
    unused(ms);
}

void __abort(char* msg) {
    void (*libc_exit)(int) = libc_sym("exit");

    if (msg != NULL) {
        void (*libc_printf)(char*, ...) = libc_sym("printf");

        libc_printf("[FAIL] %s\n", msg);
    }

    libc_exit(1);
}

void (*lox_output_string_provider)(char*) = __output_string;
void (*lox_output_char_provider)(char) = __output_char;
void* (*lox_allocate_provider)(size_t) = __malloc;
void* (*lox_aligned_allocate_provider)(size_t) = __valloc;
void (*lox_free_provider)(void*) = __free;
void* (*lox_reallocate_provider)(void*, size_t) = __realloc;
void (*lox_sleep_provider)(ulong) = __sleep;
void (*lox_abort_provider)(char*) = __abort;
