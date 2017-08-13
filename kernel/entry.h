#pragma once

#include <liblox/common.h>

extern volatile bool kernel_initialized;

does_not_return void kernel_init(void);
