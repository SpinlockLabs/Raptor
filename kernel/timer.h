#pragma once

#include <stdint.h>
#include <liblox/common.h>

extern void timer_init(uint32_t);
extern ulong timer_get_ticks(void);
extern void timer_get_relative_time(
    ulong seconds, ulong subseconds,
    ulong *out_seconds, ulong *out_subseconds);
