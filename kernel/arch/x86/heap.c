#include "heap.h"
#include "paging.h"

#include <kernel/rkmalloc/rkmalloc.h>

static rkmalloc_heap* kheap = NULL;

extern char __link_mem_end;

static spin_lock_t kheap_lock;

uintptr_t kp_placement_pointer = (uintptr_t) &__link_mem_end; // NOLINT

static volatile uintptr_t kheap_end = (uintptr_t) NULL;
uintptr_t kheap_alloc_point = KERNEL_HEAP_START;

rkmalloc_heap* heap_get(void) {
    return kheap;
}

static void* kheap_started_at = NULL;

void* heap_start(void) {
    return kheap_started_at;
}

void* kheap_allocate_align(size_t size) {
    return rkmalloc_allocate_align(kheap, size, 0x1000);
}

void* kheap_allocate(size_t size) {
    return rkmalloc_allocate(kheap, size);
}

void* kheap_reallocate(void* ptr, size_t size) {
    return rkmalloc_resize(kheap, ptr, size);
}

void kheap_free(void* ptr) {
    rkmalloc_free(kheap, ptr);
}

static uintptr_t _kpmalloc_int(size_t size, int align, uintptr_t* phys) {
    if (kheap_end != 0) {
        uintptr_t address;

        if (align) {
            address = (uintptr_t) kheap_allocate_align(size);
        } else {
            address = (uintptr_t) kheap_allocate(size);
        }

        if (phys) {
            *phys = paging_get_physical_address(address);
        }

        return address;
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

void* kpmalloc_kheap_grab_block(size_t size) {
    spin_lock(&kheap_lock);
    uintptr_t address = kheap_end;

    if (kheap_end + size > KERNEL_HEAP_END - 1) {
        panic("Kernel heap grew too much.");
    }

    if (kheap_end + size > kheap_alloc_point) {
        printf(
            INFO "Hit the end of the available kernel heap, expanding."
                " (at 0x%x, wants to be at 0x%x)\n",
            kheap_end,
            kheap_end + size
        );

        bool didExpand = false;
        for (uintptr_t i = kheap_end; i < kheap_end + size; i += 0x1000) {
            didExpand |= paging_heap_expand_into(i);
        }

        if (didExpand) {
            paging_invalidate_tables();
        }
    }

    kheap_end += size;
    spin_unlock(&kheap_lock);
    memset((void*) address, 0, size);
    return (void*) address;
}

void heap_init(void) {
    /* Initialize kheap start point. */
    kheap_end = (kp_placement_pointer + 0x1000) & ~0xFFF;
    kheap = (rkmalloc_heap*) kpmalloc_kheap_grab_block(sizeof(rkmalloc_heap));
    kheap_started_at = kheap;

    kheap->grab_block = kpmalloc_kheap_grab_block;
    kheap->return_block = NULL;

    rkmalloc_error error = rkmalloc_init_heap(kheap);

    if (error != RKMALLOC_ERROR_NONE) {
        panic("rkmalloc failed to initialize the kernel heap.");
    }
}

size_t kpused(void) {
    return kp_placement_pointer - (uint32_t) &__link_mem_end;
}

void* (*lox_allocate_provider)(size_t) = kheap_allocate;
void* (*lox_reallocate_provider)(void* ptr, size_t size) = kheap_reallocate;
void (*lox_free_provider)(void* ptr) = kheap_free;
void* (*lox_aligned_allocate_provider)(size_t) = kheap_allocate_align;
