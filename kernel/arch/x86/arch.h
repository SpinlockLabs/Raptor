#pragma once

#include <stdint.h>

typedef struct cpu_registers {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} cpu_registers_t;

typedef struct cpu_thread {
    uintptr_t esp;
    uintptr_t ebp;
    uintptr_t eip;
    uint8_t fpu_enabled;
    uint8_t fp_regs[512];
    uint8_t padding[32];
} cpu_thread_t;

typedef struct page {
    uint32_t present : 1;  // Page present in memory
    uint32_t rw : 1;  // Read-write if set
    uint32_t user : 1;  // Kernel-mode if set
    uint32_t accessed : 1;  // Page been accessed since last refresh
    uint32_t dirty : 1;  // Page been written to since last refresh
    uint32_t unused : 7;  // Unused and reserved bits
    uint32_t frame : 20; // Frame address - shifted right 12 bits
} page_t;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct {
    /**
     * Array of pointers to the page tables.
     */
    page_table_t* tables[1024];

    /**
     * Array of pointers to the page tables above,
     * gives their physical location, for loading
     * into the CR3 register.
     */
    uint32_t tablesPhysical[1024];

    /**
     * The physical address of tablesPhysical. This
     * comes into play when we get our kernel heap
     * allocated and the directory may be in a
     * different location in virtual memory.
     */
    uint32_t physicalAddr;
} page_directory_t;

typedef struct arch_process_extensions {
} arch_process_extensions_t;
