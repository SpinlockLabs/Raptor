#include "env.h"
#include "entry.h"

#include <dlfcn.h>

#include <liblox/memory.h>

#include <kernel/tty.h>
#include <kernel/time.h>

#include <time.h>

#ifndef RTLD_NEXT
#define RTLD_NEXT ((void*) -1l)
#endif

static void* libc;

int (*libc_printf)(char*, ...);
void (*libc_write)(int, uint8_t*, size_t);
void (*libc_abort)(void);
void (*libc_putc)(char);
uint (*libc_read)(int, void*, size_t);
int (*libc_ioctl)(int, ulong, ...);
void* (*libc_malloc)(size_t);
void (*libc_exit)(int);
void (*libc_free)(void*);
void* (*libc_realloc)(void*, size_t);
long (*libc_clock)(void);
struct tm* (*libc_localtime)(time_t*);

void* libc_sym(char* sym) {
    return dlsym(libc, sym);
}

void raptor_user_abort(void) {
    libc_abort();
}

void raptor_user_setup_devices(void) {
}

ulong raptor_user_ticks(void) {
    return (ulong) libc_clock();
}

void raptor_user_get_time(rtime_t* rt) {
    time_t t = time(NULL);
    struct tm* date = libc_localtime(&t);
    rt->day = (uint16_t) date->tm_mday;
    rt->month = (uint16_t) (date->tm_mon + 1);
    rt->year = (uint16_t) (date->tm_year + 1900);

    rt->hour = (uint16_t) date->tm_hour;
    rt->minute = (uint16_t) date->tm_min;
    rt->second = (uint16_t) date->tm_sec;
}

void raptor_user_console_write(tty_t* tty, const uint8_t* buffer, size_t size) {
    unused(tty);

    libc_write(1, (uint8_t*) buffer, size);
}

char* raptor_user_get_cmdline(void) {
    return "";
}

void raptor_user_output_char(char c) {
    libc_putc(c);
}

void raptor_user_output_string(char* str) {
    libc_printf("%s", str);
}

static bool raptor_user_stdin_has_data(size_t* count) {
    int res = libc_ioctl(0, 0x541B, count);
    return (res == 0 && (*count > 0));
}

void raptor_user_process_stdin(void) {
    size_t count = 0;
    if (console_tty != NULL &&
        console_tty->handle_read != NULL &&
        raptor_user_stdin_has_data(&count)) {
        uint8_t* buff = zalloc(count);
        if (libc_read(0, buff, count) > 0) {
            console_tty->handle_read(console_tty, buff, count);
        }
        free(buff);
    }
}

void* raptor_user_malloc(size_t size) {
    return libc_malloc(size);
}

void raptor_user_free(void* ptr) {
    libc_free(ptr);
}

void* raptor_user_realloc(void* ptr, size_t size) {
    return libc_realloc(ptr, size);
}

void raptor_user_exit(void) {
    libc_exit(0);
}

used void _start(void) {
    libc_malloc = dlsym(RTLD_NEXT, "malloc");

    libc = dlopen("libc.so.6", RTLD_LAZY | RTLD_LOCAL);

    if (libc == NULL) {
        int (*printf_error)(char*, ...) = dlsym(RTLD_NEXT, "printf");
        printf_error("Failed to load a libc. We need this to emulate the environment.\n");
        return;
    }

    libc_realloc = libc_sym("realloc");
    libc_free = libc_sym("free");

    libc_exit = libc_sym("exit");
    libc_read = libc_sym("read");
    libc_write = libc_sym("write");
    libc_putc = libc_sym("putc");
    libc_printf = libc_sym("printf");
    libc_abort = libc_sym("abort");
    libc_ioctl = libc_sym("ioctl");
    libc_clock = libc_sym("clock");
    libc_localtime = libc_sym("localtime");

    kernel_main();
    libc_exit(0);
}
