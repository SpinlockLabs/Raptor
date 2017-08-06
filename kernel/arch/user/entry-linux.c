#ifdef __linux__
#define __USE_GNU
#include <dlfcn.h>
#undef __USE_GNU

#include <liblox/lox-internal.h>
#include <kernel/tty.h>
#include <kernel/rkmalloc/rkmalloc.h>
#include <kernel/time.h>
#include <kernel/cpu/task.h>

#ifndef RTLD_NEXT
#define RTLD_NEXT ((void*) -1l)
#endif

#include "entry.h"

void* raptor_linux_malloc(size_t size) {
    void* (*libc_malloc)(size_t) = dlsym(RTLD_NEXT, "malloc");
    return libc_malloc(size);
}

void raptor_linux_free(void* ptr) {
    void (*libc_free)(void*) = dlsym(RTLD_NEXT, "free");
    libc_free(ptr);
}

void* raptor_linux_realloc(void* ptr, size_t size) {
    void* (*libc_realloc)(void*, size_t) = dlsym(RTLD_NEXT, "realloc");
    return libc_realloc(ptr, size);
}

void raptor_linux_console_write(tty_t* tty, const uint8_t* buffer, size_t size) {
    unused(tty);

    void (*libc_write)(int, uint8_t*, size_t) = dlsym(RTLD_NEXT, "write");
    libc_write(1, (uint8_t*) buffer, size);
}

char* raptor_linux_get_cmdline(void) {
    return "";
}

void raptor_linux_output_char(char c) {
    void (*libc_putc)(char) = dlsym(RTLD_NEXT, "putc");
    libc_putc(c);
}

void raptor_linux_output_string(char* str) {
    int (*libc_printf)(char*, ...) = dlsym(RTLD_NEXT, "printf");
    libc_printf("%s", str);
}

static tty_t* console_tty;

bool raptor_linux_stdin_has_data(size_t* count) {
    int (*libc_ioctl)(int, ulong, ...) = dlsym(RTLD_NEXT, "ioctl");
    int res = libc_ioctl(0, 0x541B, count);
    return (res == 0 && (*count > 0));
}

void raptor_linux_process_stdin(void) {
    size_t count = 0;
    if (console_tty != NULL &&
        console_tty->handle_read != NULL &&
        raptor_linux_stdin_has_data(&count)) {
        uint (*libc_read)(int, void*, size_t) = dlsym(RTLD_NEXT, "read");
        uint8_t* buff = zalloc(count);
        if (libc_read(0, buff, count) > 0) {
            console_tty->handle_read(console_tty, buff, count);
        }
        free(buff);
    }
}

void heap_init(void) {}
void paging_init(void) {}

void arch_panic_handler(char* msg) {
    if (msg != NULL) {
        puts(msg);
    }

    void (*libc_abort)(void) = dlsym(RTLD_NEXT, "abort");
    libc_abort();
}

rkmalloc_heap* heap_get(void) {
    return NULL;
}

static ulong ticks = 0;

ulong timer_get_ticks(void) {
    return ++ticks;
}

void irq_wait(void) {
}

void kernel_setup_devices(void) {
    tty_t* tty = tty_create("console");
    tty->write = raptor_linux_console_write;
    tty->flags.write_kernel_log = true;
    tty->flags.allow_debug_console = true;
    tty_register(tty);
    console_tty = tty;
}

void kernel_modules_load(void) {}

void time_get(time_t* time) {
    memset(time, 0, sizeof(time_t));
}

void cpu_run_idle(void) {
    while (true) {
        ktask_queue_flush();
        raptor_linux_process_stdin();
    }
}

used void _start(void) {
    void (*libc_exit)(int) = dlsym(RTLD_NEXT, "exit");

    kernel_main();
    libc_exit(0);
}

void* (*lox_allocate_provider)(size_t) = raptor_linux_malloc;
void* (*lox_reallocate_provider)(void*, size_t) = raptor_linux_realloc;
void (*lox_free_provider)(void*) = raptor_linux_free;
void (*lox_output_char_provider)(char) = raptor_linux_output_char;
void (*lox_output_string_provider)(char*) = raptor_linux_output_string;
char* (*arch_get_cmdline)(void) = raptor_linux_get_cmdline;
#endif
