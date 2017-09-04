#pragma once

#include <stddef.h>

#if !defined(USE_EXTERNAL_MALLOC)
/**
 * Allocates an uninitialized block of memory.
 * @param size size of block
 * @return pointer to block.
 */
void* malloc(size_t size);

/**
 * Reallocates the given block to fit the given size.
 * @param ptr pointer to original block.
 * @param size size of block
 * @return pointer to block.
 */
void* realloc(void* ptr, size_t size);

/**
 * Allocates a block of memory equivalent to (count * size).
 * @param count number of elements
 * @param size size of each element
 * @return pointer to block.
 */
void* calloc(size_t count, size_t size);

/**
 * Allocates a page-aligned block of memory.
 * @param size size of block
 * @return pointer to block.
 */
void* valloc(size_t size);

/**
 * Frees a given block of memory.
 * @param ptr pointer to block.
 */
void free(void* ptr);
#else
#include <malloc.h>
#endif

/**
 * Allocates a zero-initialized block of memory.
 * @param size size of block
 * @return pointer to block.
 */
void* zalloc(size_t size);
