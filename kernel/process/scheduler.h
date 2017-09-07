#pragma once

#include <stdbool.h>

/**
 * Yield control back to the scheduler.
 * @param reschedule whether to reschedule the current task.
 */
extern void scheduler_switch_task(bool reschedule);

/**
 * Yield control back to the scheduler and switch to a new task.
 */
extern void scheduler_switch_next(void);

/**
 * Initialize the scheduler.
 */
extern void scheduler_init(void);
