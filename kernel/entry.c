#include <liblox/common.h>
#include <liblox/io.h>

#include <kernel/cpu.h>

#include "version.h"

noreturn void kernel_init(void) {
  puts(INFO "Raptor kernel v" RAPTOR_VERSION "\n");

  cpu_run_idle();
}
