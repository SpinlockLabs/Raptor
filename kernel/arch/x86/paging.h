#pragma once

#include <stdbool.h>

#include "isr.h"

#define PAGE_SIZE (0x1000)
#define PAGE_OFFSET_MASK (PAGE_SIZE - 1)

/**
 * Allocates the page table.
 */
void paging_install(uintptr_t);

/**
 * Enables the page table.
 */
void paging_finalize(void);

/**
 * Marks a page as a system page.
 */
void paging_mark_system(uintptr_t);

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
page_t* paging_get_page(uintptr_t, int, page_directory_t*);

/**
 * Expand the kernel heap into the given address.
 * The page tables must be manually invalidated.
 */
bool paging_heap_expand_into(uintptr_t);

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
 * Adds a page mapping.
 */
uintptr_t paging_add_map(uintptr_t physical, size_t size);

/**
 * Removes a page mapping.
 */
void paging_remove_map(uintptr_t logical, size_t size);

/**
 * Retrieves the physical address for the given virtual address.
 */
uintptr_t paging_get_physical_address(uintptr_t virt);

void paging_release_directory_for_exec(page_directory_t* dir);

/**
 * Gets the amount of the used memory.
 */
uintptr_t paging_memory_used(void);

/**
 * Retrieves the current page directory.
 */
page_directory_t* paging_get_directory(void);

/**
 * Page fault handler.
 */
void page_fault(cpu_registers_t* regs);

/**
 * Allocates a page frame.
 * @param page page.
 * @param is_kernel is kernel-space
 * @param is_writable is rw
 * @return if successful
 */
bool paging_allocate_frame(page_t* page, int is_kernel, int is_writable);

/**
 * Invalidates the page cache at the given address.
 * @param addr address.
 */
void paging_invalidate_tables_at(uintptr_t addr);
