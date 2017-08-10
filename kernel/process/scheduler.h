#pragma once

#include <stdbool.h>

extern void scheduler_switch_task(bool reschedule);
extern void scheduler_switch_next(void);

extern void scheduler_init(void);
