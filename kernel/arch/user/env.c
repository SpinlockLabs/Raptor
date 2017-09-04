#include <liblox/io.h>

#include <kernel/tty.h>
#include <kernel/time.h>
#include <kernel/cpu/task.h>

#include <kernel/rkmalloc/rkmalloc.h>

#include "env.h"
#include "debug.h"

tty_t* console_tty = NULL;

#ifdef USER_RKMALLOC
static rkmalloc_heap kheap;
#endif

void heap_init(void) {
#ifdef USER_RKMALLOC
    kheap.expand = raptor_user_sbrk;
    rkmalloc_init_heap(&kheap);
#endif
}

rkmalloc_heap* heap_get(void) {
#ifdef USER_RKMALLOC
    return &kheap;
#else
    return NULL;
#endif
}

void paging_init(void) {}

void arch_panic_handler(char* msg) {
    if (msg != NULL) {
        puts(msg);
    }

    raptor_user_abort();
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
    console_tty->flags.echo = true;

    tty_register(console_tty);

    raptor_user_setup_devices();
    debug_user_init();
}

void kernel_modules_load(void) {}

void time_get(rtime_t* time) {
    raptor_user_get_time(time);
}

void raptor_user_loop(void) {
    ktask_queue_flush();
    raptor_user_process_stdin();
}

syscall_result_t raptor_user_syscall(syscall_id_t id, uintptr_t* args) {
    return 0;
}

#ifndef __EMSCRIPTEN__
void cpu_run_idle(void) {
    while (true) {
        raptor_user_loop();
    }
}
#endif

#ifdef USER_RKMALLOC
void* raptor_malloc(size_t size) {
    if (kheap.expand == NULL) {
        heap_init();
    }
    return rkmalloc_allocate(&kheap, size);
}

void* raptor_valloc(size_t size) {
    return rkmalloc_allocate(&kheap, size);
}

void* raptor_realloc(void* ptr, size_t size) {
    return rkmalloc_resize(&kheap, ptr, size);
}

void raptor_free(void* ptr) {
    rkmalloc_free(&kheap, ptr);
}

void* (*lox_allocate_provider)(size_t) = raptor_malloc;
void* (*lox_aligned_allocate_provider)(size_t) = raptor_valloc;
void* (*lox_reallocate_provider)(void*, size_t) = raptor_realloc;
void (*lox_free_provider)(void*) = raptor_free;
#else
void* (*lox_allocate_provider)(size_t) = raptor_user_malloc;
void* (*lox_aligned_allocate_provider)(size_t) = raptor_user_valloc;
void* (*lox_reallocate_provider)(void*, size_t) = raptor_user_realloc;
void (*lox_free_provider)(void*) = raptor_user_free;
#endif

void (*lox_output_char_provider)(char) = raptor_user_output_char;
void (*lox_output_string_provider)(char*) = raptor_user_output_string;
char* (*arch_get_cmdline)(void) = raptor_user_get_cmdline;
syscall_result_t (*lox_syscall_provider)(syscall_id_t, uintptr_t*) = raptor_user_syscall;
