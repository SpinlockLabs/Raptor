#include <liblox/common.h>
#include <liblox/memory.h>
#include <liblox/io.h>
#include <liblox/sleep.h>

#include <kernel/cmdline.h>
#include <kernel/cpu.h>
#include <kernel/modload.h>
#include <liblox/hex.h>

#include "paging.h"
#include "heap.h"
#include "version.h"

noreturn void kernel_init(void) {
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
