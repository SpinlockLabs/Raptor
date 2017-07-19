#include <liblox/version.h>
#include <liblox/io.h>

#include <kernel/cpu/idle.h>
#include <kernel/tty.h>
#include <kernel/modload.h>
#include <kernel/debug/console.h>

#include <liblox/lox-internal.h>

#include <kernel/disk/block.h>

#include <kernel/fs/vfs.h>

#include <kernel/network/iface.h>
#include <kernel/network/stack/stack.h>

#include <kernel/dispatch/events.h>
#include <kernel/cpu/task.h>

#include "paging.h"
#include "heap.h"
#include "rootfs.h"

/* Architecture hooks for initialization. */
extern void kernel_setup_devices(void);

extern void tty_write_kernel_log_char(char c);
extern void tty_write_kernel_log_string(char* msg);

volatile bool kernel_initialized = false;

void* (*lox_realloc_provider)(void*, size_t) = NULL;

void kernel_init(void) {
    puts(INFO "Raptor kernel v" RAPTOR_VERSION "\n");

    paging_init();
    puts(DEBUG "Paging initialized.\n");

    heap_init();
    puts(DEBUG "Heap initialized.\n");

    events_subsystem_init();
    tty_subsystem_init();
    vfs_subsystem_init();
    block_device_subsystem_init();
    network_iface_subsystem_init();
    network_stack_init();
    debug_console_init();

    lox_output_char_provider = tty_write_kernel_log_char;
    lox_output_string_provider = tty_write_kernel_log_string;

    puts(DEBUG "Loading kernel modules...\n");
    kernel_modules_load();
    puts(DEBUG "Kernel modules loaded.\n");

    kernel_setup_devices();
    debug_console_start();

    /**
     * This flag is set to tell x86 timer IRQs
     * to execute the CPU task queue.
     */
    kernel_initialized = true;

    /**
     * Flush any CPU tasks before running the
     * CPU in idle mode.
     */
    cpu_task_queue_flush();

    mount_rootfs();

    cpu_run_idle();
}
