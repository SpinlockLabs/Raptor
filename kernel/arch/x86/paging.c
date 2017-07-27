#include <liblox/common.h>
#include <liblox/hex.h>

#include <kernel/spin.h>
#include <kernel/paging.h>

#include "heap.h"
#include "irq.h"
#include "paging.h"

#define INDEX_FROM_BIT(a) ((a) / (8 * 4))
#define OFFSET_FROM_BIT(a) ((a) % (8 * 4))

static spin_lock_t frame_alloc_lock = {0};

static uintptr_t* frames;
static uintptr_t frame_count;

page_directory_t* kernel_directory;
page_directory_t* current_directory;

used static void debug_print_directory(page_directory_t* dir) {
    page_directory_t* current = dir;
    printf(
        DEBUG "Kernel Directory: 0x%x, Current Directory: 0x%x\n",
        kernel_directory, current
    );

    for (uintptr_t i = 0; i < 1024; i++) {
        if (!current->tables[i] || (uintptr_t) current->tables[i] == (uintptr_t) 0xFFFFFFFF) {
            continue;
        }

        for (uint16_t j = 0; j < 1024; j++) {
            page_t* p = &current->tables[i]->pages[j];
            if (p->frame) {
                printf(
                    DEBUG "page 0x%x 0x%x %s\n",
                    (i * 1024 + j) * 0x1000,
                    p->frame * 0x1000,
                    p->present ? "[present]" : ""
                );
            }
        }
    }
}

static void set_frame(uintptr_t frameAddr) {
    uintptr_t frame = frameAddr / 0x1000;
    uintptr_t idx = INDEX_FROM_BIT(frame);
    uintptr_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

static void clear_frame(uintptr_t frameAddr) {
    uintptr_t frame = frameAddr / 0x1000;
    uintptr_t idx = INDEX_FROM_BIT(frame);
    uintptr_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~((uintptr_t) 0x1 << off);
}

static uintptr_t test_frame(uintptr_t frameAddr) {
    uintptr_t frame = frameAddr / 0x1000;
    uintptr_t idx = INDEX_FROM_BIT(frame);
    uintptr_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}

static bool first_frame(uintptr_t* val) {
    for (uintptr_t i = 0; i < INDEX_FROM_BIT(frame_count); i++) {
        // Check for free frames.

        if (frames[i] == 0xFFFFFFFF) {
            continue;
        }

        for (uintptr_t j = 0; j < 32; j++) {
            uintptr_t toTest = (uintptr_t) (0x1 << j);

            if (frames[i] & toTest) {
                continue;
            }

            *val = i * 0x20 + j;
            return true;
        }
    }

    return false;
}

static bool alloc_frame(page_t* page, int is_kernel, int is_writable) {
    bool made = false;
    if (page->frame == 0) {
        spin_lock(frame_alloc_lock);
        uintptr_t index = 0;
        if (!first_frame(&index)) {
            panic("Out of paging frames.");
        }
        set_frame(index * 0x1000);
        page->frame = index;
        spin_unlock(frame_alloc_lock);
        made = true;
    }

    page->present = 1;
    page->rw = (is_writable == 1) ? 1 : 0;
    page->user = (is_kernel == 1) ? 0 : 1;
    return made;
}

static void dma_frame(page_t* page, int is_kernel,
                      int is_writable, uintptr_t address) {
    page->present = 1;
    page->rw = is_writable ? 1 : 0;
    page->user = is_kernel ? 0 : 1;
    page->frame = address / 0x1000;
    set_frame(address);
}

used static void free_frame(page_t* page) {
    uintptr_t frame;

    if (!(frame = page->frame)) {
        return;
    }

    clear_frame(frame * 0x1000);
    page->frame = 0;
}

void paging_install(uintptr_t memsize) {
    frame_count = memsize / 4;
    frames = (uintptr_t*) kpmalloc(INDEX_FROM_BIT(frame_count * 8));
    memset(frames, 0, INDEX_FROM_BIT(frame_count * 8));

    uintptr_t phys = 0;
    kernel_directory = (page_directory_t*) kpmalloc_ap(
        sizeof(page_directory_t), &phys
    );
    memset(kernel_directory, 0, sizeof(page_directory_t));
}

void paging_invalidate_tables(void) {
    asm volatile(
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        ::
        : "eax"
    );
}

uintptr_t paging_memory_total(void) {
    return 4 * frame_count * 0x1000;
}

uintptr_t paging_memory_used(void) {
    uintptr_t ret = 0;
    uintptr_t i, j;

    for (i = 0; i < INDEX_FROM_BIT(frame_count); i++) {
        for (j = 0; j < 32; j++) {
            uintptr_t testFrame = (uintptr_t) (0x1 << j);
            if (frames[i] & testFrame) {
                ret++;
            }
        }
    }

    return ret * 4 * 0x1000;
}

page_directory_t* paging_get_directory(void) {
    return current_directory;
}

static uintptr_t map_to_physical(uintptr_t virtual) {
    uintptr_t remaining = virtual % 0x1000;
    uintptr_t frame = virtual / 0x1000;
    uintptr_t table = frame / 1024;
    uintptr_t subframe = frame % 1024;

    if (current_directory->tables[table]) {
        page_t* p = &current_directory->tables[table]->pages[subframe];

        return p->frame * 0x1000 + remaining;
    }

    return 0;
}

static uintptr_t first_n_frames(int n) {
    for (uintptr_t i = 0; i < frame_count * 0x1000; i += 0x1000) {
        int bad = 0;

        for (int j = 0; j < n; j++) {
            if (test_frame(i + 0x1000 * j)) {
                bad = j + 1;
            }
        }

        if (!bad) {
            return i / 0x1000;
        }
    }

    return 0xFFFFFFFF;
}

uintptr_t paging_allocate_aligned_large(uintptr_t address, size_t size, uintptr_t* phys) {
    for (uintptr_t i = address; i < address + size; i += 0x1000) {
        clear_frame(map_to_physical(i));
    }

    spin_lock(frame_alloc_lock);
    uintptr_t index = first_n_frames((size + 0xFFF) / 0x1000);
    if (index == 0xFFFFFFFF) {
        spin_unlock(frame_alloc_lock);
        return 0;
    }

    for (uint i = 0; i < (size + 0xFFF) / 0x1000; i++) {
        set_frame((index + i) * 0x1000);
        page_t* page = paging_get_page(
            address + (i * 0x1000),
            0,
            kernel_directory
        );

        if (page == NULL) {
            panic("Invalid page encountered while allocating a large aligned page.");
        }

        page->frame = index + i;
        page->accessed = 1;
        page->dirty = 1;
    }

    spin_unlock(frame_alloc_lock);
    *phys = map_to_physical(address);
    return address;
}

bool paging_heap_expand_into(uintptr_t addr) {
    page_t* page = paging_get_page(addr, 0, kernel_directory);
    if (page == NULL) {
        panic("Failed to get the page while expanding kernel heap.");
    }
    return alloc_frame(page, 1, 0);
}

void paging_mark_system(uintptr_t addr) {
    set_frame(addr);
}

void paging_finalize(void) {
    paging_get_page(0, 1, kernel_directory)->present = 0;
    set_frame(0);

    /*
        Directly map the first 16MB (offset by 1MB) in the page table.
        This is split into two different tasks, the first allocating 508KB,
        then the second allocating the rest of the resulting 16MB.
    */
    for (uintptr_t i = 0x1000; i < 0x80000; i += 0x1000) {
        dma_frame(paging_get_page(i, 1, kernel_directory), 1, 0, i);
    }

    for (uintptr_t i = 0x80000; i < 0x100000; i += 0x1000) {
        dma_frame(paging_get_page(i, 1, kernel_directory), 1, 0, i);
    }

    /*
        Directly map the rest of the memory up to 3KB after the KP placement pointer.
    */
    for (uintptr_t i = 0x100000; i < kp_placement_pointer + 0x3000; i += 0x1000) {
        dma_frame(paging_get_page(i, 1, kernel_directory), 1, 0, i);
    }

    /* Allocate frames for VGA text-mode. */
    for (uintptr_t j = 0xb8000; j < 0xc0000; j += 0x1000) {
        dma_frame(paging_get_page(j, 0, kernel_directory), 0, 1, j);
    }

    /* Install the page fault handler. */
    isr_add_handler(14, page_fault);

    /* Store the physical address of the physical table. */
    kernel_directory->physicalAddr = (uintptr_t) kernel_directory->tablesPhysical;

    uintptr_t tmp_heap_start = KERNEL_HEAP_START;

    if (tmp_heap_start <= kp_placement_pointer + 0x3000) {
        printf(
            WARN "Heap start is too small: start(0x%x), placement(0x%x)\n",
            kp_placement_pointer + 0x3000
        );
        tmp_heap_start = kp_placement_pointer + 0x100000;
        kheap_alloc_point = tmp_heap_start;
    }

    /* Allocate starting kernel heap. */
    for (uintptr_t i = kp_placement_pointer + 0x3000; i < tmp_heap_start; i += 0x1000) {
        alloc_frame(paging_get_page(i, 1, kernel_directory), 1, 0);
    }

    /* Create pages for an extended heap allocation. */
    for (uintptr_t i = tmp_heap_start; i < KERNEL_HEAP_END; i += 0x1000) {
        paging_get_page(i, 1, kernel_directory);
    }

    current_directory = paging_clone_directory(kernel_directory);
    paging_switch_directory(kernel_directory);
}

void paging_switch_directory(page_directory_t* dir) {
    current_directory = dir;

    asm volatile(
        "mov %0, %%cr3\n"
        "mov %%cr0, %%eax\n"
        "orl $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n"
        ::"r"(dir->physicalAddr)
        : "eax"
    );
}

page_t* paging_get_page(uintptr_t address, int make, page_directory_t* dir) {
    address /= 0x1000;

    uintptr_t table_idx = address / 1024;
    if (dir->tables[table_idx]) {
        return &dir->tables[table_idx]->pages[address % 1024];
    }

    if (make) {
        uintptr_t tmp = 0;
        dir->tables[table_idx] = (page_table_t*) kpmalloc_ap(
            sizeof(page_table_t),
            &tmp
        );
        memset(dir->tables[table_idx], 0, sizeof(page_table_t));
        dir->tablesPhysical[table_idx] = tmp | 0x7; /*  Present, RW, User */
        return &dir->tables[table_idx]->pages[address % 1024];
    }

    return NULL;
}

void paging_map_dma(uintptr_t virt, uintptr_t phys) {
    page_t* page = paging_get_page(virt, 1, kernel_directory);
    if (page == NULL) {
        return;
    }

    dma_frame(page, 1, 1, phys);
}

void paging_unmap_dma(uintptr_t virt) {
    page_t* page = paging_get_page(virt, 0, kernel_directory);
    if (page == NULL) {
        return;
    }

    alloc_frame(page, 1, 1);
}

uintptr_t paging_get_physical_address(uintptr_t virt) {
    return map_to_physical(virt);
}

uintptr_t paging_add_map(uintptr_t physical, size_t size) {
    size = (size_t) (((size / 0x1000) + 0.5) * 0x1000);
    uintptr_t ptr = kpmalloc_a(size);
    for (uintptr_t addr = ptr; addr < ptr + size; addr += 0x1000) {
        paging_map_dma(addr, physical + (addr - ptr));
    }
    return ptr;
}

void paging_remove_map(uintptr_t logical, size_t size) {
    unused(logical);
    unused(size);
}

void page_fault(cpu_registers_t* regs) {
    int_disable();
    uintptr_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    int present = !(regs->err_code & 0x1) ? 1 : 0;
    int rw = (regs->err_code & 0x2) ? 1 : 0;
    int us = (regs->err_code & 0x4) ? 1 : 0;
    int reserved = (regs->err_code & 0x8) ? 1 : 0;
    int id = (regs->err_code & 0x10) ? 1 : 0;

    uintptr_t eip = regs->eip;

    printf(
        ERROR
            "Segmentation fault ("
            "present: %d, "
            "rw: %d, "
            "user: %d, "
            "reserved: %d, "
            "id: %d, "
            "address: 0x%x, "
            "caused by: 0x%x)\n",
        present,
        rw,
        us,
        reserved,
        id,
        faulting_address,
        eip
    );

    print_registers(regs);

    panic(NULL);
}

extern void copy_page_physical(uintptr_t, uintptr_t);

page_table_t* paging_clone_table(page_table_t* src, uintptr_t* phys) {
    page_table_t* table = (page_table_t*) kpmalloc_ap(sizeof(page_table_t), phys);
    memset(table, 0, sizeof(page_table_t));
    uintptr_t i;
    for (i = 0; i < 1024; ++i) {
        if (!src->pages[i].frame) {
            continue;
        }
        alloc_frame(&table->pages[i], 0, 0);
        if (src->pages[i].present) {
            table->pages[i].present = 1;
        }

        if (src->pages[i].rw) {
            table->pages[i].rw = 1;
        }

        if (src->pages[i].user) {
            table->pages[i].user = 1;
        }

        if (src->pages[i].dirty) {
            table->pages[i].dirty = 1;
        }

        if (src->pages[i].accessed) {
            table->pages[i].accessed = 1;
        }

        copy_page_physical(src->pages[i].frame * 0x1000, table->pages[i].frame * 0x1000);
    }
    return table;
}

page_directory_t* paging_clone_directory(page_directory_t* src) {
    uintptr_t phys;
    page_directory_t* out = (page_directory_t*) kpmalloc_ap(
        sizeof(page_directory_t),
        &phys
    );

    memset(out, 0, sizeof(page_directory_t));
    out->physicalAddr = phys;

    for (uintptr_t i = 0; i < 1024; i++) {
        if (!src->tables[i] ||
            (uintptr_t) src->tables[i] == (uintptr_t) 0xFFFFFFFF) {
            continue;
        }

        if (kernel_directory->tables[i] == src->tables[i]) {
            out->tables[i] = src->tables[i];
            out->tablesPhysical[i] = src->tablesPhysical[i];
        } else {
            out->tables[i] = paging_clone_table(src->tables[i], &phys);
            out->tablesPhysical[i] = phys | 0x07;
        }
    }

    return out;
}
