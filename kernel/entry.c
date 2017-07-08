#include <liblox/io.h>

#include <kernel/cpu/idle.h>
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
