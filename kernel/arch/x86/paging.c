#include "heap.h"
#include "paging.h"
#include "irq.h"

#include <liblox/string.h>

uint32_t *frames;
uint32_t frame_count;

page_directory_t* kernel_directory;
page_directory_t* current_directory;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

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

static uint32_t test_frame(uint32_t frameAddr) {
    uint32_t frame = frameAddr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}

static uint32_t first_frame() {
    for (uint32_t i = 0; i < INDEX_FROM_BIT(frame_count); i++) {
        // Check for free frames
        if (frames[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                uint32_t toTest = 0x1 << j;
                if (!(frames[i] & toTest)) {
                    return i * 4 * 8 + j;
                }
            }
        }
    }
}

void alloc_frame(page_t* page, int isKernel, int isWritable) {
    if (page->frame != 0) {
        return;
    } else {
        uint32_t idx = first_frame();
        if (idx == (uint32_t) -1) {
            // TODO: Panic
        }
        set_frame(idx * 0x1000);
        page->present = 1;
        page->rw = isWritable ? 1 : 0;
        page->user = isKernel ? 0 : 1;
        page->frame = idx;
    }
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
    frames = (uint32_t*) kmalloc(INDEX_FROM_BIT(frame_count));
    memset(frames, 0, INDEX_FROM_BIT(frame_count));

    kernel_directory = (page_directory_t*) kmalloc_a(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));
    current_directory = kernel_directory;

    int i = 0;
    while (i < placement_address) {
        alloc_frame(paging_get_page(i, 1, kernel_directory), 0, 0);
        i += 0x1000;
    }

    irq_add_handler(14, page_fault);

    paging_switch_directory(kernel_directory);
}

void paging_switch_directory(page_directory_t* dir) {
    current_directory = dir;
    asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

page_t* paging_get_page(uint32_t address, int make, page_directory_t *dir) {
    address /= 0x1000;
    uint32_t table_idx = address / 1024;
    if (dir->tables[table_idx]) {
        return &dir->tables[table_idx]->pages[address % 1024];
    } else if (make) {
        uint32_t tmp;
        dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
        memset(dir->tables[table_idx], 0, 0x1000);
        dir->tablesPhysical[table_idx] = tmp | 0x7;
        return &dir->tables[table_idx]->pages[address%1024];
    } else {
        return 0;
    }
}

void page_fault(regs_t regs) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    int present = !(regs.err_code & 0x1);
    int rw = regs.err_code & 0x2;
    int us = regs.err_code & 0x4;
    int reserved = regs.err_code & 0x8;
    int id = regs.err_code & 0x10;

    puts("Page fault (");
    if (present) puts("present ");
    if (rw) puts("rw ");
    if (us) puts("usermode ");
    if (reserved) puts("reserved ");
    puts(") at 0x");
    puts(faulting_address);
    puts("\n");
    // TODO: Panic
    asm volatile("cli; hlt");
}
