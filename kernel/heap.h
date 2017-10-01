#pragma once

#include <kernel/rkmalloc/rkmalloc.h>

arch_specific void heap_init(void);
arch_specific rkmalloc_heap* heap_get(void);
