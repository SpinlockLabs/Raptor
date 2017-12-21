#pragma once

#include <liblox/common.h>

/**
 * Architecture hooks for initialization.
 */
arch_specific void kernel_setup_devices(void);

/**
 * Initializes the kernel, and task switches into the scheduler.
 */
does_not_return void kernel_init(void);

/**
 * Checks if the kernel is initialized.
 * 
 * Returns true after the kernel is fully initialized.
 */
bool kernel_is_initialized(void);
