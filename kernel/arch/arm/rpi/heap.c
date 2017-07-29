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

    memset((void*) addr, 0, size);
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
    kheap->expand = rkpmalloc;

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

void kheap_free(void *ptr) {
    rkmalloc_free(kheap, ptr);
}

void* kheap_reallocate(void* ptr, size_t size) {
    return rkmalloc_resize(kheap, ptr, size);
}

void* (*lox_allocate_provider)(size_t) = kheap_allocate;
void (*lox_free_provider)(void *ptr) = kheap_free;
void* (*lox_reallocate_provider)(void* ptr, size_t size) = kheap_reallocate;
