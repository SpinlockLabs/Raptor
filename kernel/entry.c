#include <liblox/io.h>

#include <kernel/debug/console.h>
#include <kernel/cpu/idle.h>
#include <kernel/modload.h>
#include <kernel/tty.h>

#include <liblox/lox-internal.h>

#include "paging.h"
#include "heap.h"
#include "version.h"
#include "cmdline.h"

/* Architecture hooks for initialization. */
extern void post_subsystem_init(void);

extern void tty_write_kernel_log_char(char c);
extern void tty_write_kernel_log_string(char* msg);

void kernel_init(void) {
    puts(INFO "Raptor kernel v" RAPTOR_VERSION "\n");

    paging_init();
    puts(DEBUG "Paging initialized.\n");

    heap_init();
    puts(DEBUG "Heap initialized.\n");

    tty_subsystem_init();
    debug_console_init();
    post_subsystem_init();

    lox_output_char_provider = tty_write_kernel_log_char;
    lox_output_string_provider = tty_write_kernel_log_string;

    puts(DEBUG "Loading kernel modules...\n");
    kernel_modules_load();
    puts(DEBUG "Kernel modules loaded.\n");

    debug_console_start();

    cpu_run_idle();
}
