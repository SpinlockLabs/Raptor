#pragma once

#include <liblox/common.h>

extern volatile bool kernel_initialized;

/* Architecture hooks for initialization. */
arch_specific void kernel_setup_devices(void);

does_not_return void kernel_init(void);
