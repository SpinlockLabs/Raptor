#include <kernel/spin.h>

#include <liblox/hex.h>

#include "rkmalloc.h"

#ifndef RKMALLOC_DISABLE_MAGIC
static uintptr_t rkmagic(uintptr_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}
#endif

void rkmalloc_init_heap(rkmalloc_heap* heap) {
#define CHKSIZE(size) \
    if ((size) == 0) { \
        heap->error_code = RKMALLOC_ERROR_TYPE_TOO_SMALL; \
        return; \
    }

    if (heap->expand == NULL) {
        heap->error_code = RKMALLOC_ERROR_INVALID_POINTER;
        return;
    }

    heap->error_code = RKMALLOC_ERROR_NONE;
    heap->total_allocated_blocks_size = 0;
    heap->total_allocated_used_size = 0;
    spin_init(heap->lock);

    list_init(&heap->index);

    CHKSIZE(heap->types.atomic)
    CHKSIZE(heap->types.molecular)
    CHKSIZE(heap->types.nano)
    CHKSIZE(heap->types.micro)
    CHKSIZE(heap->types.mini)
    CHKSIZE(heap->types.tiny)
    CHKSIZE(heap->types.small)
    CHKSIZE(heap->types.medium)
    CHKSIZE(heap->types.moderate)
    CHKSIZE(heap->types.fair)
    CHKSIZE(heap->types.large)
    CHKSIZE(heap->types.huge)
}

static size_t get_block_size(rkmalloc_heap_types types, size_t size) {
    if (size <= types.atomic) {
        return types.atomic;
    }

    if (size <= types.molecular) {
        return types.molecular;
    }

    if (size <= types.nano) {
        return types.nano;
    }

    if (size <= types.micro) {
        return types.micro;
    }

    if (size <= types.mini) {
        return types.mini;
    }

    if (size <= types.tiny) {
        return types.tiny;
    }

    if (size <= types.small) {
        return types.small;
    }

    if (size <= types.medium) {
        return types.medium;
    }

    if (size <= types.moderate) {
        return types.moderate;
    }

    if (size <= types.fair) {
        return types.fair;
    }

    if (size <= types.large) {
        return types.large;
    }

    if (size <= types.huge) {
        return types.huge;
    }

    return size;
}

static bool is_block_usable(rkmalloc_entry* entry, size_t block_size) {
    if (!entry->free) {
        return false;
    }

    if (block_size == entry->block_size) {
        return true;
    }

    return false;
}

void* rkmalloc_allocate(rkmalloc_heap* heap, size_t size) {
    if (size == 0) {
        return NULL;
    }

    spin_lock(heap->lock);

    size_t block_size = get_block_size(heap->types, size);
    list_node_t* node = heap->index.head;

    while (node != NULL && !is_block_usable(node->value, block_size)) {
        node = node->next;
    }

    /*
     * Our best case is that we find a node in the index that can fit the size.
     */
    if (node != NULL) {
        rkmalloc_entry* entry = node->value;
        entry->free = false;
        entry->used_size = size;
        heap->total_allocated_blocks_size += entry->block_size;
        heap->total_allocated_used_size += size;

        spin_unlock(heap->lock);

        return entry->ptr;
    }

    /* TODO(kaendfinger): Implement combining blocks. */

    size_t header_and_size =
        sizeof(list_node_t) + sizeof(rkmalloc_entry) + block_size;

    node = heap->expand(header_and_size);

    if (node == NULL) {
        spin_unlock(heap->lock);
        return NULL;
    }

    list_init_node(node);
    node->list = &heap->index;

    rkmalloc_entry* entry = (rkmalloc_entry*) ((void*) node + sizeof(list_node_t));

    entry->free = false;
    entry->block_size = block_size;
    entry->used_size = size;
    entry->ptr = ((void*) entry + sizeof(rkmalloc_entry));

#ifndef RKMALLOC_DISABLE_MAGIC
    entry->magic = rkmagic((uintptr_t) entry->ptr);
#endif

    heap->total_allocated_blocks_size += block_size;
    heap->total_allocated_used_size += size;

    node->value = entry;

    list_add_node(&heap->index, node);

    spin_unlock(heap->lock);

    return entry->ptr;
}

void rkmalloc_free(rkmalloc_heap* heap, void* ptr) {
    if (ptr == NULL) {
        return;
    }

#ifndef RKMALLOC_DISABLE_MAGIC
    rkmalloc_entry* entry = (rkmalloc_entry*) (ptr - sizeof(rkmalloc_entry));
    list_node_t* node = (list_node_t*) (entry - sizeof(list_node_t));

    if (entry->magic != rkmagic((uintptr_t) ptr)) {
        puts(WARN "Attempted to free an invalid pointer (bad magic header) (");
        puthex((int) ptr);
        puts(")\n");
    }
#else
    list_node_t* node = heap->index.head;
    rkmalloc_entry* entry = NULL;
    while (node != NULL) {
        entry = node->value;
        if (entry->ptr == ptr) {
            break;
        }

        node = node->next;
    }
#endif

    if (node == NULL) {
        puts(WARN "Attempted to free an invalid pointer (");
        puthex((int) ptr);
        puts(")\n");
    } else if (entry->free) {
        puts(WARN "Attempted to free an already freed pointer (");
        puthex((int) ptr);
        puts(")\n");
    } else {
        entry->free = true;
        heap->total_allocated_blocks_size -= entry->block_size;
        heap->total_allocated_used_size -= entry->used_size;
    }
}
