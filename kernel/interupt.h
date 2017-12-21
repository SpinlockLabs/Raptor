#pragma once

#include <kernel/common.h>

/**
 * Wait for the next interrupt.
 */
arch_specific void irq_wait(void);
