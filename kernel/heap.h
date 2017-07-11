#pragma once

#include <kernel/rkmalloc/rkmalloc.h>

void heap_init(void);
rkmalloc_heap* heap_get(void);
