#include <assert.h>
#include "heap.h"

extern char __link_mem_end;
uint32_t placement_address = (uint32_t) &__link_mem_end;

static uint32_t _kmalloc_int(uint32_t size, int align, uint32_t *phys) {
    if (align && (placement_address & 0xFFFFF000)) {
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000;
    }
    if (phys) {
        *phys = placement_address;
    }
    uint32_t addr = placement_address;
    placement_address += size;
    return addr;
}

uint32_t kmalloc_a(uint32_t size) {
    return _kmalloc_int(size, 1, 0);
}

uint32_t kmalloc_p(uint32_t size, uint32_t *phys) {
    return _kmalloc_int(size, 0, phys);
}

uint32_t kmalloc_ap(uint32_t size, uint32_t *phys) {
    return _kmalloc_int(size, 1, phys);
}

uint32_t kmalloc(uint32_t size) {
    return _kmalloc_int(size, 0, 0);
}

static int32_t find_smallest_hole(uint32_t size, uint8_t pageAlign, heap_t *heap) {
    uint32_t iterator = 0;
    while (iterator < heap->index.size) {
        header_t *header = (header_t *) list_get(iterator, &heap->index);
        if (pageAlign > 0) {
            // Page-align the starting point of this header.
            uint32_t location = (uint32_t) header;
            int32_t offset = 0;
            if ((location + sizeof(header_t)) & 0xFFFFF000 != 0) {
                offset = 0x1000 /* page size */ - (location + sizeof(header_t) % 0x1000);
            }
            int32_t holeSize = (int32_t) header->size - offset;

            // Can it fit? ;)
            if (holeSize >= (int32_t) size) {
                break;
            }
        } else if (header->size >= size) {
            break;
        }
        iterator++;
    }
    if (iterator == heap->index.size) {
        return -1; // It wouldn't fit :'(
    } else {
        return iterator;
    }
}

static int8_t header_t_less_than(void *a, void *b) {
    return (((header_t *) a)->size < ((header_t *)b)->size) ? 1 : 0;
}

heap_t *create_heap(uint32_t start, uint32_t end, uint32_t max, uint8_t supervisor, uint8_t readonly) {
    heap_t *heap = (heap_t *) kmalloc(sizeof(heap_t));

    assert(start % 0x1000 == 0);
    assert(end % 0x1000 == 0);

    heap->index = list_create((void *) start, HEAP_INDEX_SIZE, &header_t_less_than);
}
