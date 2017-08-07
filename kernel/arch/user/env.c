#include <liblox/io.h>

#include <kernel/tty.h>
#include <kernel/rkmalloc/rkmalloc.h>
#include <kernel/time.h>
#include <kernel/cpu/task.h>

#include "env.h"
#include "debug.h"

tty_t* console_tty = NULL;

void heap_init(void) {}
void paging_init(void) {}

void arch_panic_handler(char* msg) {
    if (msg != NULL) {
        puts(msg);
    }

    raptor_user_abort();
}

rkmalloc_heap* heap_get(void) {
    return NULL;
}

ulong timer_get_ticks(void) {
    return raptor_user_ticks();
}

void irq_wait(void) {
}

void kernel_setup_devices(void) {
    console_tty = tty_create("console");
    console_tty->write = raptor_user_console_write;
    console_tty->flags.write_kernel_log = true;
    console_tty->flags.allow_debug_console = true;

#ifndef __unix__
    console_tty->flags.echo = true;
#endif

    tty_register(console_tty);

    debug_user_init();
}

void kernel_modules_load(void) {}

void time_get(rtime_t* time) {
    raptor_user_get_time(time);
}

void cpu_run_idle(void) {
    while (true) {
        ktask_queue_flush();
        raptor_user_process_stdin();
    }
}

void* (*lox_allocate_provider)(size_t) = raptor_user_malloc;
void* (*lox_reallocate_provider)(void*, size_t) = raptor_user_realloc;
void (*lox_free_provider)(void*) = raptor_user_free;
void (*lox_output_char_provider)(char) = raptor_user_output_char;
void (*lox_output_string_provider)(char*) = raptor_user_output_string;
char* (*arch_get_cmdline)(void) = raptor_user_get_cmdline;
