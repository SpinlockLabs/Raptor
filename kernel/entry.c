#include <liblox/common.h>

#include <liblox/memory.h>
#include <liblox/io.h>
#include <liblox/sleep.h>
#include <liblox/string.h>
#include <liblox/hex.h>

#include <kernel/cpu/idle.h>
#include <kernel/cpu/task.h>

#include <kernel/cmdline.h>
#include <kernel/timer.h>
#include <kernel/modload.h>

#include "paging.h"
#include "heap.h"
#include "version.h"

void kernel_init(void) {
    puts(INFO "Raptor kernel v" RAPTOR_VERSION "\n");

    paging_init();
    puts(DEBUG "Paging initialized.\n");

    heap_init();
    puts(DEBUG "Heap initialized.\n");

    puts(DEBUG "Loading kernel modules...\n");
    kernel_modules_load();
    puts(DEBUG "Kernel modules loaded.\n");

    cpu_run_idle();
}
