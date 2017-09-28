#include "scheduler.h"

#ifndef ARCH_HAS_SCHEDULER
#include <liblox/common.h>
#include <kernel/cpu/idle.h>

void scheduler_switch_next(void) {
    cpu_run_idle();
}

void scheduler_switch_task(bool reschedule) {
    unused(reschedule);
    scheduler_switch_next();
}

void scheduler_init(void) {
}
#endif
