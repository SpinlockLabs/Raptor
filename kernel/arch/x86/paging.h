#pragma once

#include <stdbool.h>

#include "isr.h"

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

/**
 * Allocates the page table.
 */
void paging_install(uint32_t);

/**
 * Enables the page table.
 */
void paging_finalize(void);

/**
 * Marks a page as a system page.
 */
void paging_mark_system(uint64_t);

/**
 * Invalidate the page tables.
 */
void paging_invalidate_tables(void);

/**
 * Loads a specific page into the CR3 register.
 */
void paging_switch_directory(page_directory_t*);

/**
 * Retrieves a pointer to the given page.
 */
page_t* paging_get_page(uint32_t, int, page_directory_t*);

/**
 * Expand the kernel heap into the given address.
 * The page tables must be manually invalidated.
 */
void paging_heap_expand_into(uint32_t);

/**
 * Allocates a large aligned page for use with some devices.
 * Stores the physical address in the given pointer.
 */
uintptr_t paging_allocate_aligned_large(uintptr_t, size_t, uintptr_t*);

/**
 * Gets the total amount of memory.
 */
uintptr_t paging_memory_total(void);

/**
 * Directly maps a memory page to physical memory.
 */
void paging_map_dma(uintptr_t virt, uintptr_t phys);

/**
 * Unmaps a memory page from physical memory.
 */
void paging_unmap_dma(uintptr_t);

/**
 * Retrieves the physical address for the given virtual address.
 */
uintptr_t paging_get_physical_address(uintptr_t virt);

/**
 * Gets the amount of the used memory.
 */
uintptr_t paging_memory_used(void);

/**
 * Page fault handler.
 */
void page_fault(regs_t regs);
