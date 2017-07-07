#include "heap.h"

#include <kernel/rkmalloc/rkmalloc.h>

static rkmalloc_heap *kheap = NULL;

extern char __link_mem_end;

uint32_t kheap_placement_address = (uint32_t) &__link_mem_end;

static uint32_t _kpmalloc_int(uint32_t size, int align, uint32_t *phys) {
    if (align && (kheap_placement_address & 0xFFFFF000)) {
        kheap_placement_address &= 0xFFFFF000;
        kheap_placement_address += 0x1000;
    }

    uint32_t addr = kheap_placement_address;

    if (phys) {
        *phys = addr;
    }

    kheap_placement_address += size;
    return addr;
}

uint32_t kpmalloc_a(uint32_t size) {
    return _kpmalloc_int(size, 1, 0);
}

uint32_t kpmalloc_p(uint32_t size, uint32_t *phys) {
    return _kpmalloc_int(size, 0, phys);
}

uint32_t kpmalloc_ap(uint32_t size, uint32_t *phys) {
    return _kpmalloc_int(size, 1, phys);
}

uint32_t kpmalloc(uint32_t size) {
    return _kpmalloc_int(size, 0, 0);
}

void* rkpmalloc(size_t size) {
    uint32_t p = 0;
    return (void*) kpmalloc_ap(size, &p);
}

void heap_init(void) {
    kheap = (rkmalloc_heap*) kpmalloc(sizeof(rkmalloc_heap));
    kheap->kmalloc = rkpmalloc;
    kheap->types = (rkmalloc_heap_types) {
        .tiny = 1 * 1024, // 1 kb
        .small = 2 * 1024, // 2 KB
        .medium = 16 * 1024, // 16 kb
        .large = 1024 * 1024, // 1 mb
        .huge = 5 * 1024 * 1024 // 5 mb
    };

    rkmalloc_init_heap(kheap);
}

void* kheap_allocate(size_t size) {
    return rkmalloc_allocate(kheap, size);
}

void kheap_free(void *ptr) {
    rkmalloc_free(kheap, ptr);
}

void* (*lox_allocate_provider)(size_t) = kheap_allocate;
void (*lox_free_provider)(void *ptr) = kheap_free;
