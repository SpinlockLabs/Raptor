#pragma once

#include <liblox/common.h>
#include <stdint.h>

#define KERNEL_HEAP_START 0x00800000
#define KERNEL_HEAP_END 0x20000000

extern uintptr_t kp_placement_pointer;
extern uintptr_t kheap_alloc_point;

uintptr_t kpmalloc_a(size_t);
uintptr_t kpmalloc_p(size_t, uintptr_t*);
uintptr_t kpmalloc_ap(size_t, uintptr_t*);
uintptr_t kpmalloc(size_t);

void kpmalloc_startat(uintptr_t);
size_t kpused(void);

void heap_init(void);
