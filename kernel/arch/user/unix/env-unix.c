#include "../env.h"
#include "../entry.h"

#include <dlfcn.h>

#include <unistd.h>
#include <termios.h>

#ifdef __APPLE__
#include "../mac/init.h"
#elif __linux__
#include "../linux/init.h"
#endif

#include <liblox/memory.h>

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
void* (*libc_valloc)(size_t);
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
    if (isatty(STDIN_FILENO)) {
        struct termios tattr;
        tcgetattr(STDIN_FILENO, &tattr);
        tattr.c_lflag &= ~(ICANON | ECHO);
        tattr.c_cc[VMIN] = 1;
        tattr.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
    } else {
        console_tty->flags.echo = false;
    }

#ifdef __APPLE__
    raptor_user_mac_init();
#elif __linux__
    raptor_user_linux_init();
#endif
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
#ifdef __APPLE__
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    *count = 1024;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
#else
    int res = libc_ioctl(0, 0x541B, count);
    return (res == 0 && (*count > 0));
#endif
}

void raptor_user_process_stdin(void) {
    size_t count = 0;
    if (console_tty != NULL &&
        console_tty->handle_read != NULL &&
        raptor_user_stdin_has_data(&count)) {
        uint8_t* buff = zalloc(count);
        int ret = libc_read(0, buff, count);
        if (ret > 0) {
#ifdef __APPLE__
            count = ret;
#endif
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

void* raptor_user_valloc(size_t size) {
    return libc_valloc(size);
}

void raptor_user_exit(void) {
    libc_exit(0);
}

#ifdef __APPLE__
#define LIBC_NAME "libc.dylib"
#else
#define LIBC_NAME "libc.so.6"
#endif

used void _start(void) {
    libc_malloc = dlsym(RTLD_NEXT, "malloc");

    libc = dlopen(LIBC_NAME, RTLD_LAZY | RTLD_LOCAL);

    if (libc == NULL) {
        int (*printf_error)(char*, ...) = dlsym(RTLD_NEXT, "printf");
        printf_error("Failed to load a libc. We need this to emulate the environment.\n");
        return;
    }

    libc_valloc = libc_sym("valloc");
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

#ifdef __APPLE__
int main(void) {
    _start();
    return 0;
}
#endif
