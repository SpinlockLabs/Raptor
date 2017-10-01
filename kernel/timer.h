#pragma once

#include <stdint.h>
#include <liblox/common.h>

extern arch_specific void timer_init(uint32_t);
extern arch_specific ulong timer_get_ticks(void);
