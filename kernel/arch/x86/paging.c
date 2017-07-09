#include <liblox/common.h>
#include <liblox/hex.h>

#include "heap.h"
#include "irq.h"
#include "paging.h"

uint32_t* frames;
uint32_t frame_count;

page_directory_t* kernel_directory;
page_directory_t* current_directory;

#define INDEX_FROM_BIT(a) ((a) / (8 * 4))
#define OFFSET_FROM_BIT(a) ((a) % (8 * 4))

static void set_frame(uint32_t frameAddr) {
    uint32_t frame = frameAddr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

static void clear_frame(uint32_t frameAddr) {
    uint32_t frame = frameAddr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

used static uint32_t test_frame(uint32_t frameAddr) {
    uint32_t frame = frameAddr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}

static int first_frame(uint32_t* val) {
    for (uint32_t i = 0; i < INDEX_FROM_BIT(frame_count); i++) {
        // Check for free frames.

        if (frames[i] == 0xFFFFFFFF) {
            continue;
        }

        for (uint32_t j = 0; j < 32; j++) {
            uint32_t toTest = (uint32_t) (0x1 << j);

            if (frames[i] & toTest) {
                continue;
            }

            *val = i * 4 * 8 + j;
            return 0;
        }
    }
    return 1;
}

void alloc_frame(page_t* page, int isKernel, int isWritable) {
    if (page->frame != 0) {
        return;
    }

    uint32_t idx = 0;

    if (first_frame(&idx) != 0) {
        panic("First frame in paging is invalid.");
    }

    set_frame(idx * 0x1000);
    page->present = 1;
    page->rw = isWritable ? 1 : 0;
    page->user = isKernel ? 0 : 1;
    page->frame = idx;
}

void free_frame(page_t* page) {
    if (page->frame == 0) {
        return;
    }
    clear_frame(page->frame);
    page->frame = 0;
}

void paging_init(void) {
    uint32_t memEndPage = 0x10000000;
    frame_count = memEndPage / 0x1000;
    frames = (uint32_t*) kpmalloc(INDEX_FROM_BIT(frame_count));
    memset(frames, 0, INDEX_FROM_BIT(frame_count));

    kernel_directory = (page_directory_t*) kpmalloc_a(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));
    current_directory = kernel_directory;

    uint32_t i = 0;
    while (i < kheap_placement_address) {
        alloc_frame(paging_get_page(i, 1, kernel_directory), 0, 0);
        i += 0x1000;
    }

    uintptr_t tmp_heap_start = kheap_init_address;

    for (i = kheap_placement_address + 0x3000; i < tmp_heap_start; i += 0x1000) {
        alloc_frame(paging_get_page(i, 1, kernel_directory), 1, 0);
    }

    for (i = tmp_heap_start; i < kheap_end_address; i += 0x1000) {
        paging_get_page(i, 1, kernel_directory);
    }

    isr_add_handler(14, (irq_handler_t) page_fault);

    paging_switch_directory(kernel_directory);
}

void paging_switch_directory(page_directory_t* dir) {
    current_directory = dir;
    asm volatile("mov %0, %%cr3"::"r"(&dir->tablesPhysical));
    uint32_t cr0 = 0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0"::"r"(cr0));
}

page_t* paging_get_page(uint32_t address, int make, page_directory_t* dir) {
    address /= 0x1000;
    uint32_t table_idx = address / 1024;
    if (dir->tables[table_idx]) {
        return &dir->tables[table_idx]->pages[address % 1024];
    }

    if (make) {
        uint32_t tmp;
        dir->tables[table_idx] = (page_table_t*) kpmalloc_ap(sizeof(page_table_t), &tmp);
        memset(dir->tables[table_idx], 0, 0x1000);
        dir->tablesPhysical[table_idx] = tmp | 0x7;
        return &dir->tables[table_idx]->pages[address % 1024];
    }

    return NULL;
}

void page_fault(regs_t regs) {
    int_disable();
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    int present = !(regs.err_code & 0x1);
    int rw = regs.err_code & 0x2;
    int us = regs.err_code & 0x4;
    int reserved = regs.err_code & 0x8;
    // int id = regs.err_code & 0x10;

    puts(ERROR "Segmentation fault (");
    if (present) {
        puts(" present");
    }

    if (rw) {
        puts(" rw");
    }

    if (us) {
        puts(" usermode");
    }

    if (reserved) {
        puts(" reserved");
    }

    puts(" ) at ");
    puthex((int) faulting_address);
    puts(" by ");
    puthex((int) regs.eip);
    puts("\n");

    panic(NULL);
}
