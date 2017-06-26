#pragma once

#include "isr.h"
#include <stdbool.h>

typedef struct {
    uint32_t present : 1;  // Page present in memory
    uint32_t rw : 1;  // Read-write if set
    uint32_t user : 1;  // Kernel-mode if set
    uint32_t accessed : 1;  // Page been accessed since last refresh
    uint32_t dirty : 1;  // Page been written to since last refresh
    uint32_t unused : 7;  // Unused and reserved bits
    uint32_t frame : 20; // Frame address - shifted right 12 bits
} page_t;

typedef struct {
    page_t pages[1024];
} page_table_t;

typedef struct {
    /**
     * Array of pointers to the page tables.
     */
    page_table_t *tables[1024];

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

/**
 * Initializes the page table and enables it.
 */
void paging_init(void);

/**
 * Loads a specific page into the CR3 register.
 */
void paging_switch_directory(page_directory_t *);

/**
 * Retrieves a pointer to the page
 */
page_t *paging_get_page(uint32_t, int, page_directory_t *);

/**
 * Page fault handler.
 */
void page_fault(regs_t regs);
