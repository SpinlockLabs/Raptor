#include <kernel/cpu.h>

#include "irq.h"

void cpu_run_idle(void) {
    while (true) {
        int_enable();
        asm("hlt;");
    }
}
