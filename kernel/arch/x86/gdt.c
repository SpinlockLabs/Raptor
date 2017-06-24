#include "gdt.h"

// Limit, shortens code in gdt_init.
#define L 0xFFFFFFFF

void gdt_init(void) {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    // Data segments
    gdt_set_gate(0, 0, 0, 0, 0);       // Null
    gdt_set_gate(1, 0, L, 0x9A, 0xCF); // Code
    gdt_set_gate(2, 0, L, 0x92, 0xCF); // Data
    gdt_set_gate(3, 0, L, 0xFA, 0xCF); // User mode code
    gdt_set_gate(4, 0, L, 0xF2, 0xCF); // User mode data

    gdt_flush((uint32_t)&gdt_ptr);
}

void gdt_set_gate(int32_t num, uint32_t base, uint32_t lim, uint8_t acc, uint8_t gran) {
    gdt_entries[num].base_low = (uint16_t) (base & 0xFFFF);
    gdt_entries[num].base_middle = (uint8_t) ((base >> 16) & 0xFF);
    gdt_entries[num].base_high = (uint8_t) ((base >> 24) & 0xFF);

    gdt_entries[num].limit_low = (uint16_t) (lim & 0xFFFF);
    gdt_entries[num].granularity = (uint8_t) ((lim >> 16) & 0x0F);

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access = acc;
}

