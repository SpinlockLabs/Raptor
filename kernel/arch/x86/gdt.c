#include <liblox/string.h>

#include "gdt.h"

// Limit, shortens code in gdt_init.
#define L 0xFFFFFFFF

static struct {
    gdt_entry_t entries[6];
    gdt_pointer_t pointer;
    tss_entry_t tss;
} gdt;

#define ENTRY(X) (gdt.entries[(X)])

static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0) {
    tss_entry_t* tss = &gdt.tss;

    uintptr_t base = (uintptr_t) tss;
    uintptr_t limit = base + sizeof(*tss);

    /* Add the TSS descriptor to the GDT */
    gdt_set_gate(num, base, limit, 0xE9, 0x00);

    memset(tss, 0x0, sizeof(*tss));

    gdt.tss.ss0 = ss0;
    gdt.tss.esp0 = esp0;
    gdt.tss.cs = 0x0b;
    gdt.tss.ss = 0x13;
    gdt.tss.ds = 0x13;
    gdt.tss.es = 0x13;
    gdt.tss.fs = 0x13;
    gdt.tss.gs = 0x13;

    gdt.tss.iomap_base = sizeof(*tss);
}

void gdt_init(void) {
    gdt_pointer_t* gdtp = &gdt.pointer;
    gdtp->limit = sizeof(gdt.entries) - 1;
    gdtp->base = (uintptr_t) &ENTRY(0);

    // Data segments
    gdt_set_gate(0, 0, 0, 0, 0);       // Null
    gdt_set_gate(1, 0, L, 0x9A, 0xCF); // Code
    gdt_set_gate(2, 0, L, 0x92, 0xCF); // Data
    gdt_set_gate(3, 0, L, 0xFA, 0xCF); // User mode code
    gdt_set_gate(4, 0, L, 0xF2, 0xCF); // User mode data

    write_tss(5, 0x10, 0x0);

    gdt_flush((uint32_t) gdtp);
    tss_flush();
}

void gdt_set_gate(int32_t num, uint32_t base, uint32_t lim, uint8_t acc, uint8_t gran) {
    ENTRY(num).base_low = (uint16_t) (base & 0xFFFF);
    ENTRY(num).base_middle = (uint8_t) ((base >> 16) & 0xFF);
    ENTRY(num).base_high = (uint8_t) ((base >> 24) & 0xFF);

    ENTRY(num).limit_low = (uint16_t) (lim & 0xFFFF);
    ENTRY(num).granularity = (uint8_t) ((lim >> 16) & 0x0F);

    ENTRY(num).granularity |= gran & 0xF0;
    ENTRY(num).access = acc;
}

void set_kernel_stack(uintptr_t stack) {
    /* Set the kernel stack */
    gdt.tss.esp0 = stack;
}
