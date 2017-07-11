#include "heap.h"
#include "paging.h"

#include <kernel/rkmalloc/rkmalloc.h>

static rkmalloc_heap* kheap = NULL;

extern char __link_mem_end;

static spin_lock_t kheap_lock = {0};

uintptr_t kp_placement_pointer = (uintptr_t) &__link_mem_end;
volatile uintptr_t kheap_end = (uintptr_t) NULL;
uintptr_t kheap_alloc_point = KERNEL_HEAP_START;

static uintptr_t _kpmalloc_int(size_t size, int align, uintptr_t* phys) {
    if (kheap_end != 0) {
        if (phys && align && size >= 0x3000) {
            printf(WARN "Allocating a large aligned buffer. This may take a second.\n");
            spin_lock(kheap_lock);
            uintptr_t addr = kheap_end;
            addr &= 0xFFFFF000;
            addr += 0x1000;
            kheap_end = addr + size;
            spin_unlock(kheap_lock);
            return paging_allocate_aligned_large(addr, addr, phys);
        }

        printf(WARN "kpmalloc called after heap was initialized!\n");
        return (uintptr_t) NULL;
    }

    if (align && (kp_placement_pointer & 0xFFFFF000)) {
        kp_placement_pointer &= 0xFFFFF000;
        kp_placement_pointer += 0x1000;
    }

    uint32_t addr = kp_placement_pointer;

    if (phys) {
        *phys = addr;
    }

    kp_placement_pointer += size;
    return addr;
}

uintptr_t kpmalloc_a(size_t size) {
    return _kpmalloc_int(size, 1, 0);
}

uintptr_t kpmalloc_p(size_t size, uintptr_t* phys) {
    return _kpmalloc_int(size, 0, phys);
}

uintptr_t kpmalloc_ap(size_t size, uintptr_t* phys) {
    return _kpmalloc_int(size, 1, phys);
}

uintptr_t kpmalloc(size_t size) {
    return _kpmalloc_int(size, 0, 0);
}

void kpmalloc_start_at(uintptr_t addr) {
    kp_placement_pointer = addr;
}

void* kpmalloc_kheap_expand(size_t size) {
    spin_lock(kheap_lock);
    uintptr_t address = kheap_end;

    if (kheap_end + size > kheap_alloc_point) {
        printf(
            INFO "Hit the end of the available kernel heap, expanding."
                " (at 0x%x, wants to be at 0x%x)\n",
            kheap_end,
            kheap_end + size
        );

        for (uintptr_t i = kheap_end; i < kheap_end + size; i += 0x1000) {
            paging_heap_expand_into(i);
        }
        paging_invalidate_tables();
    }

    kheap_end += size;
    spin_unlock(kheap_lock);
    memset((void*) address, 0, size);
    return (void*) address;
}

void heap_init(void) {
    /* Initialize kheap start point. */
    kheap_end = (kp_placement_pointer + 0x1000) & ~0xFFF;
    kheap = (rkmalloc_heap*) kpmalloc_kheap_expand(sizeof(rkmalloc_heap));

    kheap->expand = kpmalloc_kheap_expand;
    kheap->types.atomic = 8; // 8 bytes
    kheap->types.molecular = 16; // 16 bytes
    kheap->types.nano = 64; // 64 bytes
    kheap->types.micro = 256; // 256 bytes
    kheap->types.mini = 512; // 512 bytes
    kheap->types.tiny = 1 * 1024; // 1 kb
    kheap->types.small = 2 * 1024; // 2 kb
    kheap->types.medium = 4 * 1024; // 4 kb
    kheap->types.moderate = 16 * 1024; // 16 kb
    kheap->types.fair = 64 * 1024; // 64 kb
    kheap->types.large = 1024 * 1024; // 1 mb
    kheap->types.huge = 5 * 1024 * 1024; // 5 mb

    rkmalloc_init_heap(kheap);
}

rkmalloc_heap* heap_get(void) {
    return kheap;
}

void* kheap_allocate(size_t size) {
    return rkmalloc_allocate(kheap, size);
}

void kheap_free(void* ptr) {
    rkmalloc_free(kheap, ptr);
}

size_t kpused(void) {
    return kp_placement_pointer - (uint32_t) &__link_mem_end;
}

void* (*lox_allocate_provider)(size_t) = kheap_allocate;
void (*lox_free_provider)(void* ptr) = kheap_free;
