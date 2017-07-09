#pragma once

#include <liblox/common.h>
#include <stdint.h>

extern uintptr_t kheap_placement_address;
extern uintptr_t kheap_init_address;
extern uint kheap_end_address;

uintptr_t kpmalloc_a(size_t);
uintptr_t kpmalloc_p(size_t, uintptr_t*);
uintptr_t kpmalloc_ap(size_t, uintptr_t*);
uintptr_t kpmalloc(size_t);

void kpmalloc_startat(uintptr_t);
size_t kpused(void);

void heap_init(void);
