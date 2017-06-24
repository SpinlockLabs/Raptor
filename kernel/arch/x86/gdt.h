#pragma once

#include <stdint.h>

typedef struct {
    uint16_t limit_low;   // Lower 16 bits of the limit.
    uint16_t base_low;    // Lower 16 bits of the base.
    uint8_t  base_middle; // Next 8 bits of the base.
    uint8_t  access;      // Access flags, ring the segment can be used in.
    uint8_t  granularity;
    uint8_t  base_high;   // Last 8 bits of the base.
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;       // Upper 16 bits of all selector limits.
    uint32_t base;        // Address of the first gdt_entry_t struct.
} __attribute__((packed)) gdt_ptr_t;

gdt_entry_t gdt_entries[5];
gdt_ptr_t gdt_ptr;

void gdt_init(void);
void gdt_set_gate(int32_t, uint32_t, uint32_t, uint8_t, uint8_t);

// Reloads new segment registers.
extern void gdt_flush(int);
