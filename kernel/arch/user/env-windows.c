#ifdef _WIN32

#include "env.h"
#include "entry.h"

#include <windows.h>

static HMODULE libc;
static HMODULE kernel32;

void* libc_sym(char* sym) {
    return GetProcAddress(libc, sym);
}

void* raptor_user_malloc(size_t size) {
    void* (*libc_malloc)(size_t) = libc_sym("malloc");
    return libc_malloc(size);
}

void raptor_user_free(void* ptr) {
    void (*libc_free)(void*) = libc_sym("free");
    libc_free(ptr);
}

void* raptor_user_realloc(size_t size, void* ptr) {
    void* (*libc_realloc)(size_t, void*) = libc_sym("realloc");
    return libc_realloc(size, ptr);
}

void raptor_user_abort(void) {
    void(*libc_abort)(void) = libc_sym("abort");
    libc_abort();
}

char* raptor_user_get_cmdline(void) {
    char* (*getcmdline)(void) = (void*) GetProcAddress(kernel32, "GetCommandLineA");
    return getcmdline();
}

void raptor_user_console_write(tty_t* tty, const uint8_t* buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        char c = (char) buffer[i];
        if (c == '\r') {
            raptor_user_output_char('\r');
            c = '\n';
        }
        raptor_user_output_char(c);
    }
}

void raptor_user_output_string(char* msg) {
    int (*libc_printf)(char*, ...) = libc_sym("printf");
    libc_printf("%s", msg);
}

void raptor_user_output_char(char c) {
    int (*libc_printf)(char*, ...) = libc_sym("printf");
    libc_printf("%c", c);
}

void raptor_user_process_stdin(void) {
    int (*kbhit)(void) = libc_sym("_kbhit");
    char (*getch)(void) = libc_sym("_getch");

    if (kbhit()) {
        char c = getch();
        if (console_tty != NULL && console_tty->handle_read != NULL) {
            console_tty->handle_read(console_tty, &c, 1);
        }
    }
}

int WINAPI mainCRTStartup(void) {
    libc = LoadLibrary("msvcrt.dll");
    kernel32 = LoadLibrary("kernel32.dll");

    kernel_main();
    return 0;
}

#endif
