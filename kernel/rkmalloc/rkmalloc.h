/* A custom memory allocator for the Raptor Kernel */
#pragma once

#include <stdint.h>

#include <liblox/common.h>
#include <liblox/string.h>
#include <liblox/list.h>

#include <kernel/spin.h>
#include <kernel/panic.h>

#ifndef RKMALLOC_SITTER_COUNT
#define RKMALLOC_SITTER_COUNT 16
#endif

typedef void* (*rkmalloc_expand_func_t)(size_t);

/**
 * An heap entry for rkmalloc.
 * This structure is packed to reduce the size as much as possible.
 */
typedef struct rkmalloc_entry {
#ifndef RKMALLOC_DISABLE_MAGIC
    uintptr_t magic;
#endif
    union {
        struct {
            bool free : 1;
            bool sitting : 1;
        };

        uint32_t flags;
    };
    size_t used_size;
    size_t block_size;
    void* ptr;
} packed rkmalloc_entry;

/**
 * A heap index entry.
 */
typedef struct rkmalloc_index_entry {
    list_node_t node;
    rkmalloc_entry entry;
    uint8_t ptr[0];
} packed rkmalloc_index_entry;

typedef struct {
    size_t atomic;
    size_t molecular;
    size_t nano;
    size_t micro;
    size_t mini;
    size_t tiny;
    size_t small;
    size_t medium;
    size_t moderate;
    size_t fair;
    size_t large;
    size_t huge;
} rkmalloc_heap_types;

typedef enum {
    RKMALLOC_ERROR_NONE = 0,
    RKMALLOC_ERROR_TYPE_TOO_SMALL = 1,
    RKMALLOC_ERROR_INVALID_POINTER = 2,
    RKMALLOC_ERROR_FAILED_TO_ALLOCATE = 3
} rkmalloc_error;

typedef struct rkmalloc_heap {
    rkmalloc_expand_func_t expand;
    rkmalloc_error error_code;
    size_t total_allocated_blocks_size;
    size_t total_allocated_used_size;
    list_t index;
    rkmalloc_heap_types types;
    spin_lock_t lock;
    rkmalloc_entry* sitters[RKMALLOC_SITTER_COUNT];
} rkmalloc_heap;

void rkmalloc_init_heap(rkmalloc_heap* heap);
void* rkmalloc_allocate(rkmalloc_heap* heap, size_t size);
void* rkmalloc_resize(rkmalloc_heap* heap, void* ptr, size_t new_size);
void rkmalloc_free(rkmalloc_heap* heap, void* ptr);

#ifndef RKMALLOC_DISABLE_MAGIC
uintptr_t rkmagic(uintptr_t x);
#endif
