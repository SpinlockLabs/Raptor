#pragma once

#include <liblox/common.h>

extern volatile bool kernel_initialized;

noreturn void kernel_init(void);
