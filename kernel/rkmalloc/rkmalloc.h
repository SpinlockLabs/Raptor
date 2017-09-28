/* A custom memory allocator for the Raptor Kernel */
#pragma once

#include <stdint.h>

#include <liblox/common.h>
#include <liblox/string.h>
#include <liblox/list.h>

#include <kernel/spin.h>

#ifndef RKMALLOC_SITTER_COUNT
#define RKMALLOC_SITTER_COUNT 16
#endif

/**
 * A function that, given a size,
 * will return a new block of memory.
 * @param size requested size
 * @return block
 */
typedef void* (*rkmalloc_grab_block_func_t)(
    size_t size
);

/**
 * A function that can be called when
 * the heap is ready to return a block
 * of allocated memory.
 * @param block block
 */
typedef void (*rkmalloc_return_block_func_t)(
    void* block
);

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
} packed rkmalloc_entry;

/**
 * A heap index entry.
 */
typedef struct rkmalloc_index_entry {
    list_node_t node;
    rkmalloc_entry entry;
    uint8_t ptr[];
} packed rkmalloc_index_entry;

typedef enum {
    RKMALLOC_ERROR_NONE = 0,
    RKMALLOC_ERROR_TYPE_TOO_SMALL = 1,
    RKMALLOC_ERROR_INVALID_POINTER = 2
} rkmalloc_error;

typedef struct rkmalloc_heap {
    rkmalloc_grab_block_func_t grab_block;
    rkmalloc_return_block_func_t return_block;

    size_t total_allocated_blocks_size;
    size_t total_allocated_used_size;
    list_t index;

    spin_lock_t lock;
    rkmalloc_entry* sitters[RKMALLOC_SITTER_COUNT];
} rkmalloc_heap;

/**
 * Initialize a heap.
 * @param heap heap instance
 * @return error code
 */
rkmalloc_error rkmalloc_init_heap(rkmalloc_heap* heap);

/**
* Allocate a memory block.
* @param heap rkmalloc heap
* @param size requested size
* @return block
*/
void* rkmalloc_allocate(rkmalloc_heap* heap, size_t size);

/**
 * Allocate an aligned memory block.
 * @param heap rkmalloc heap
 * @param size requested size
 * @param align alignment
 * @return block
 */
void* rkmalloc_allocate_align(rkmalloc_heap* heap, size_t size, size_t align);

/**
 * Reserve a block of the given size in the heap.
 * The block is automatically added to the free list.
 * @param heap rkmalloc heap
 * @param size block size
 */
void rkmalloc_reserve(rkmalloc_heap* heap, size_t size);

/**
 * Resize a memory block to a different size.
 * @param heap rkmalloc heap
 * @param ptr block
 * @param new_size requested size
 * @return new block
 */
void* rkmalloc_resize(rkmalloc_heap* heap, void* ptr, size_t new_size);

/**
 * Mark a memory block as free.
 * @param heap rkmalloc heap
 * @param ptr block
 */
void rkmalloc_free(rkmalloc_heap* heap, void* ptr);

/**
 * Attempt to return free blocks.
 * @param heap rkmalloc heap
 * @param aggressive whether to be more aggressive
 * @return number of blocks freed
 */
uint rkmalloc_reduce(
    rkmalloc_heap* heap,
    bool aggressive
);

#ifndef RKMALLOC_DISABLE_MAGIC
/**
 * Calculate the magic hash of a given pointer.
 * This used to verify the heap for validity.
 * @param input input address
 * @return magic hash
 */
uintptr_t rkmagic(uintptr_t input);
#endif
