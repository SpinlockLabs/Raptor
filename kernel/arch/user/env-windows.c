#ifdef _WIN32

#include "env.h"
#include "entry.h"

#include <windows.h>

static HMODULE libc;
static HMODULE kernel32;

void* (*libc_malloc)(size_t);
void(*libc_free)(void*);
void* (*libc_realloc)(size_t, void*);
void (*libc_abort)(void);
void (*libc_exit)(int);

int (*libc_printf)(char*, ...);

void* libc_sym(char* sym) {
    return GetProcAddress(libc, sym);
}

void* raptor_user_malloc(size_t size) {
    return libc_malloc(size);
}

void raptor_user_free(void* ptr) {
    libc_free(ptr);
}

void* raptor_user_realloc(size_t size, void* ptr) {
    return libc_realloc(size, ptr);
}

ulong raptor_user_ticks(void) {
    static ulong ticks = 0;
    return ++ticks;
}

void raptor_user_get_time(rtime_t* time) {
    memset(time, 0, sizeof(rtime_t));
}

void raptor_user_abort(void) {
    libc_abort();
}

void raptor_user_exit(void) {
    libc_exit(0);
}

char* raptor_user_get_cmdline(void) {
    char* (*getcmdline)(void) = (void*) GetProcAddress(
        kernel32,
        "GetCommandLineA"
    );
    return getcmdline();
}

void raptor_user_console_write(
    tty_t* tty,
    const uint8_t* buffer,
    size_t size
) {
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
    libc_printf("%s", msg);
}

void raptor_user_output_char(char c) {
    libc_printf("%c", c);
}

int (*kbhit)(void);
char (*getch)(void);

void raptor_user_process_stdin(void) {
    if (kbhit()) {
        char c = getch();
        if (console_tty != NULL &&
            console_tty->handle_read != NULL) {
            console_tty->handle_read(
                console_tty,
                (uint8_t*) &c,
                1
            );
        }
    }
}

// ReSharper disable once CppInconsistentNaming
int WINAPI mainCRTStartup(void) {
    libc = LoadLibrary("msvcrt.dll");
    kernel32 = LoadLibrary("kernel32.dll");

    libc_malloc = libc_sym("malloc");
    libc_free = libc_sym("free");
    libc_realloc = libc_sym("realloc");
    libc_abort = libc_sym("abort");
    libc_exit = libc_sym("exit");

    libc_printf = libc_sym("printf");

    kbhit = libc_sym("_kbhit");
    getch = libc_sym("_getch");

    kernel_main();
    return 0;
}

#endif
