#include <kernel/rkmalloc/rkmalloc.h>

static rkmalloc_heap *kheap = NULL;

extern char __end;

uint32_t kheap_placement_address = (uint32_t) &__end;

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

void* kheap_grab_block(size_t size) {
    void* ptr = (void*) kpmalloc(size);
    memset(ptr, 0, size);
    return ptr;
}

void heap_init(void) {
    kheap = (rkmalloc_heap*) kpmalloc(sizeof(rkmalloc_heap));
    kheap->grab_block = kheap_grab_block;
    kheap->return_block = NULL;

    rkmalloc_init_heap(kheap);
}

rkmalloc_heap* heap_get(void) {
    return kheap;
}

void* kheap_allocate(size_t size) {
    return rkmalloc_allocate(kheap, size);
}

void* kheap_allocate_align(size_t size) {
    return rkmalloc_allocate_align(kheap, size, 0x1000);
}

void kheap_free(void *ptr) {
    rkmalloc_free(kheap, ptr);
}

void* kheap_reallocate(void* ptr, size_t size) {
    return rkmalloc_resize(kheap, ptr, size);
}

void* (*lox_allocate_provider)(size_t) = kheap_allocate;
void* (*lox_aligned_allocate_provider)(size_t) = kheap_allocate_align;
void (*lox_free_provider)(void *ptr) = kheap_free;
void* (*lox_reallocate_provider)(void* ptr, size_t size) = kheap_reallocate;
