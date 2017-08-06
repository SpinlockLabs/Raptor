#ifdef __unix__

#include <dlfcn.h>

#include <kernel/tty.h>

#include "env.h"
#include "entry.h"

#ifndef RTLD_NEXT
#define RTLD_NEXT ((void*) -1l)
#endif

static void* libc;

void* libc_sym(char* sym) {
    return dlsym(libc, sym);
}

void raptor_user_abort(void) {
    void (*libc_abort)(void) = libc_sym("abort");
    libc_abort();
}

void raptor_user_console_write(tty_t* tty, const uint8_t* buffer, size_t size) {
    unused(tty);

    void (*libc_write)(int, uint8_t*, size_t) = libc_sym("write");
    libc_write(1, (uint8_t*) buffer, size);
}

char* raptor_user_get_cmdline(void) {
    return "";
}

void raptor_user_output_char(char c) {
    void (*libc_putc)(char) = libc_sym("putc");
    libc_putc(c);
}

void raptor_user_output_string(char* str) {
    int (*libc_printf)(char*, ...) = libc_sym("printf");
    libc_printf("%s", str);
}

static bool raptor_user_stdin_has_data(size_t* count) {
    int (*libc_ioctl)(int, ulong, ...) = libc_sym("ioctl");
    int res = libc_ioctl(0, 0x541B, count);
    return (res == 0 && (*count > 0));
}

void raptor_user_process_stdin(void) {
    size_t count = 0;
    if (console_tty != NULL &&
        console_tty->handle_read != NULL &&
        raptor_user_stdin_has_data(&count)) {
        uint (*libc_read)(int, void*, size_t) = libc_sym("read");
        uint8_t* buff = zalloc(count);
        if (libc_read(0, buff, count) > 0) {
            console_tty->handle_read(console_tty, buff, count);
        }
        free(buff);
    }
}

void* raptor_user_malloc(size_t size) {
    void* (*libc_malloc)(size_t) = dlsym(RTLD_NEXT, "malloc");
    return libc_malloc(size);
}

void raptor_user_free(void* ptr) {
    void (*libc_free)(void*) = dlsym(libc, "free");
    libc_free(ptr);
}

void* raptor_user_realloc(void* ptr, size_t size) {
    void* (*libc_realloc)(void*, size_t) = libc_sym("realloc");
    return libc_realloc(ptr, size);
}

used void _start(void) {
    libc = dlopen("libc.so.6", RTLD_LAZY | RTLD_LOCAL);

    if (libc == NULL) {
        int (*libc_printf)(char*, ...) = dlsym(RTLD_NEXT, "printf");
        libc_printf("Failed to load a libc. We need this to emulate the environment.\n");
        return;
    }

    void (*libc_exit)(int) = libc_sym("exit");

    kernel_main();
    libc_exit(0);
}
#endif
