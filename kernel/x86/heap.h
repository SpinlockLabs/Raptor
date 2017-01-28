#pragma once
#include <stdint.h>

uint32_t placement_address;

uint32_t kmalloc_a(uint32_t);
uint32_t kmalloc_p(uint32_t, uint32_t *);
uint32_t kmalloc_ap(uint32_t, uint32_t *);
uint32_t kmalloc(uint32_t);

