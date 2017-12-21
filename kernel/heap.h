#pragma once

#include <kernel/rkmalloc/rkmalloc.h>

/**
 * Initializes the kernel heap.
 */
arch_specific void heap_init(void);

/**
 * Retrieves the kernel heap.
 */
arch_specific rkmalloc_heap* heap_get(void);
