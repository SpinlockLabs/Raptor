/* A custom memory allocator for the Raptor Kernel */
#pragma once

#include <stdint.h>

#include <liblox/common.h>
#include <liblox/string.h>
#include <liblox/list.h>

#include <kernel/panic.h>

typedef void* (*kmalloc_func_t)(size_t size);

typedef struct {
    size_t tiny;
    size_t small;
    size_t medium;
    size_t large;
    size_t huge;
} rkmalloc_heap_types;

typedef enum {
    RKMALLOC_ERROR_NONE = 0,
    RKMALLOC_ERROR_TYPE_TOO_SMALL = 1,
    RKMALLOC_ERROR_INVALID_POINTER = 2,
    RKMALLOC_ERROR_FAILED_TO_ALLOCATE = 3
} rkmalloc_error;

typedef struct {
    kmalloc_func_t kmalloc;
    rkmalloc_error error_code;
    size_t total_allocated_blocks_size;
    size_t total_allocated_used_size;
    list_t index;
    rkmalloc_heap_types types;
} rkmalloc_heap;

extern uint8_t rkmalloc_error_code;

void rkmalloc_init_heap(rkmalloc_heap *heap);
void* rkmalloc_allocate(rkmalloc_heap *heap, size_t size);
void rkmalloc_free(rkmalloc_heap *heap, void *ptr);
