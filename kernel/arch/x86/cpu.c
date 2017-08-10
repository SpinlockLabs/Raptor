#include <kernel/cpu/idle.h>
#include <kernel/cpu/task.h>

#include "irq.h"

void cpu_run_idle(void) {
    while (true) {
        ktask_queue_flush();
        int_enable();
        asm("hlt;");
    }
}
