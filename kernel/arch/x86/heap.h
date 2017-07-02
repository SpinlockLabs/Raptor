#pragma once

#include <liblox/common.h>
#include <stdint.h>

extern uint32_t kheap_placement_address;

uint32_t kpmalloc_a(uint32_t);
uint32_t kpmalloc_p(uint32_t, uint32_t *);
uint32_t kpmalloc_ap(uint32_t, uint32_t *);
uint32_t kpmalloc(uint32_t);

void heap_init(void);
