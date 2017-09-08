#include <liblox/version.h>
#include <liblox/io.h>

#include <kernel/tty.h>
#include <kernel/modload.h>
#include <kernel/debug/console.h>

#include <liblox/lox-internal.h>

#include <kernel/disk/block.h>

#include <kernel/fs/vfs.h>

#include <kernel/network/iface.h>
#include <kernel/network/stack/stack.h>

#include <kernel/process/process.h>

#include <kernel/dispatch/events.h>
#include <kernel/cpu/task.h>
#include <kernel/process/scheduler.h>
#include <kernel/syscall/table.h>
#include <kernel/syscall/lox/loxcall.h>

#include "paging.h"
#include "heap.h"
#include "rootfs.h"

/* Architecture hooks for initialization. */
extern void kernel_setup_devices(void);

extern void tty_write_kernel_log_char(char c);
extern void tty_write_kernel_log_string(char* msg);

volatile bool kernel_initialized = false;

void kernel_init(void) {
    puts(INFO "Raptor kernel v" RAPTOR_VERSION "\n");

    paging_init();
    puts(DEBUG "Paging initialized.\n");

    heap_init();
    puts(DEBUG "Heap initialized.\n");

    device_registry_init();
    printf(DEBUG "Device registry initialized.\n");

    events_subsystem_init();
    printf(DEBUG "Event system initialized.\n");

    tty_subsystem_init();
    printf(DEBUG "Console initialized.\n");

    vfs_subsystem_init();
    block_device_subsystem_init();
    printf(DEBUG "VFS initialized.\n");

    network_iface_subsystem_init();
    network_stack_init();
    printf(DEBUG "Network initialized.\n");

    syscall_init();
    syscall_loxcall_init();

    process_tree_init();
    scheduler_init();

    debug_console_init();
    printf(DEBUG "Debug console initialized.\n");
    kernel_setup_devices();

    event_dispatch(EVENT_DRIVER_SETUP, NULL);

    lox_output_char_provider = tty_write_kernel_log_char;
    lox_output_string_provider = tty_write_kernel_log_string;

    puts(DEBUG "Loading kernel modules...\n");
    kernel_modules_load();
    puts(DEBUG "Kernel modules loaded.\n");

    debug_console_start();

    /**
     * This flag is set to tell x86 timer IRQs
     * to execute the CPU task queue.
     */
    kernel_initialized = true;

    /**
     * Flush any kernel tasks before running the
     * CPU in idle mode.
     */
    ktask_queue_flush();

    mount_rootfs();

    scheduler_switch_next();
}
