#pragma once

#include <stdint.h>
#include <liblox/common.h>

void timer_init(uint32_t);
uint32_t timer_get_ticks(void);
