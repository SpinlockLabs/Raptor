#include "heap.h"

#include <kernel/rkmalloc/rkmalloc.h>

rkmalloc_heap* kheap = NULL;

extern char __link_mem_end;

uintptr_t kheap_placement_address = (uintptr_t) &__link_mem_end;
uintptr_t kheap_init_address = 0x00800000;
uintptr_t kheap_end_address = 0x20000000;

static uintptr_t _kpmalloc_int(size_t size, int align, uintptr_t* phys) {
    if (align && (kheap_placement_address & 0xFFFFF000)) {
        kheap_placement_address &= 0xFFFFF000;
        kheap_placement_address += 0x1000;
    }

    uint32_t addr = kheap_placement_address;

    if (addr + size > kheap_end_address) {
        return (uintptr_t) NULL;
    }

    if (phys) {
        *phys = addr;
    }

    kheap_placement_address += size;
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

void kpmalloc_startat(uintptr_t addr) {
    kheap_placement_address = addr;
}

void* rkpmalloc(size_t size) {
    return (void*) kpmalloc_a(size);
}

void heap_init(void) {
    kheap = (rkmalloc_heap*) kpmalloc(sizeof(rkmalloc_heap));
    memset(kheap, 0, sizeof(rkmalloc_heap));
    kheap->kmalloc = rkpmalloc;

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

void* kheap_allocate(size_t size) {
    return rkmalloc_allocate(kheap, size);
}

void kheap_free(void* ptr) {
    rkmalloc_free(kheap, ptr);
}

size_t kpused(void) {
    return kheap_placement_address - (uint32_t) &__link_mem_end;
}

void* (*lox_allocate_provider)(size_t) = kheap_allocate;
void (*lox_free_provider)(void* ptr) = kheap_free;
