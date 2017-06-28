#pragma once

#include <liblox/common.h>
#include <stdint.h>

#define KHEAP_START 0xC0000000
#define KHEAP_INITIAL_SIZE 0x100000
#define KHEAP_INDEX_SIZE 0x20000
#define HEAP_MAGIX 0x123890AB
#define HEAP_MIN_SIZE 0x70000

extern uint32_t placement_address;

typedef struct {
    uint32_t magic; // Magic number, used for error checking and identification
    uint8_t hole; // 1 if hole, 0 if block
    uint32_t size; // Size of the block, with header and footer
} packed header_t;

typedef struct {
    uint32_t magic; // Magic number, used for error checking and identification
    header_t *header; // Pointer to header
} packed footer_t;

typedef struct {
    ordered_array_t index;
    uint32_t start_address; // The start of our allocated space.
    uint32_t end_address; // The end of our allocated space, may be expanded up to max_address;
    uint32_t max_address; // The maximum address the heap can be expanded to.
    uint8_t supervisor; // Should extra pages requested be mapped as supervisor only?
    uint8_t readonly; // Should extra pages request be mapped as readonly?
} packed heap_t;

heap_t *create_heap(uint32_t start, uint32_t end, uint32_t max, uint8_t supervisor, uint8_t readonly);
void *kalloc(uint32_t size, uint8_t page_align, heap_t *heap);
void kfree(void *p, heap_t *heap);

uint32_t kmalloc_a(uint32_t);
uint32_t kmalloc_p(uint32_t, uint32_t *);
uint32_t kmalloc_ap(uint32_t, uint32_t *);
uint32_t kmalloc(uint32_t);
