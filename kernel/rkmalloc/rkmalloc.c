#include <kernel/spin.h>

#include <liblox/hex.h>

#include "rkmalloc.h"

#ifndef RKMALLOC_DISABLE_MAGIC
uintptr_t rkmagic(uintptr_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}
#endif

rkmalloc_error rkmalloc_init_heap(rkmalloc_heap* heap) {
#define CHKSIZE(size) \
    if ((size) == 0) { \
        return RKMALLOC_ERROR_TYPE_TOO_SMALL; \
    }

    if (heap->expand == NULL) {
        return RKMALLOC_ERROR_INVALID_POINTER;
    }

    heap->total_allocated_blocks_size = 0;
    heap->total_allocated_used_size = 0;
    spin_init(&heap->lock);

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
    return RKMALLOC_ERROR_NONE;
}

static list_node_t* get_pointer_entry(rkmalloc_heap* heap, void* ptr, rkmalloc_entry** eout) {
#ifndef RKMALLOC_DISABLE_MAGIC
    unused(heap);

    rkmalloc_entry* entry = (rkmalloc_entry*) ((uintptr_t) ptr - sizeof(rkmalloc_entry));
    list_node_t* node = (list_node_t*) ((uintptr_t) entry - sizeof(list_node_t));

    if (entry->magic != rkmagic((uintptr_t) ptr)) {
        *eout = NULL;
        return NULL;
    }
    *eout = entry;
#else
    spin_lock(&heap->lock);
    list_node_t* node = heap->index.head;
    rkmalloc_entry* entry = NULL;
    while (node != NULL) {
        entry = node->value;
        rkmalloc_index_entry* index = (rkmalloc_index_entry*) node;
        if (&index->ptr == ptr) {
            break;
        }

        node = node->next;
    }
    spin_unlock(&heap->lock);
    *eout = entry;
#endif

    return node;
}

static void insert_sitter(rkmalloc_heap* heap, rkmalloc_entry* entry) {
    for (uint i = 0; i < RKMALLOC_SITTER_COUNT; i++) {
        if (heap->sitters[i] == NULL) {
            heap->sitters[i] = entry;
            entry->sitting = true;
            return;
        }
    }

    rkmalloc_entry* prev = heap->sitters[0];
    heap->sitters[0] = entry;
    prev->sitting = false;
}

static void drop_sitter(rkmalloc_heap* heap, rkmalloc_entry* entry) {
    for (uint i = 0; i < RKMALLOC_SITTER_COUNT; i++) {
        if (heap->sitters[i] == entry) {
            heap->sitters[i] = NULL;
            entry->sitting = false;
            return;
        }
    }
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

void* rkmalloc_allocate_align(rkmalloc_heap* heap, size_t size, size_t align) {
    size_t mask = align - 1;
    size_t true_size = size + align;

    if ((get_block_size(heap->types, size) - size) >= align) {
        true_size = size;
    }

    uintptr_t result = (uintptr_t) rkmalloc_allocate(heap, true_size);

    if (result == 0) {
        return (void*) result;
    }

    if ((result % align) == 0) {
        return (void*) result;
    }

    return (void*) ((result + mask) & ~mask);
}

void* rkmalloc_allocate(rkmalloc_heap* heap, size_t size) {
    if (size == 0) {
        return NULL;
    }

    spin_lock(&heap->lock);

    size_t block_size = get_block_size(heap->types, size);

    rkmalloc_entry* entry = NULL;

    for (uint i = 0; i < RKMALLOC_SITTER_COUNT; i++) {
        rkmalloc_entry* candidate = heap->sitters[i];
        if (candidate != NULL && is_block_usable(candidate, block_size)) {
            entry = candidate;
            break;
        }
    }

    if (entry == NULL) {
        list_node_t* node = heap->index.head;

        while (node != NULL && !is_block_usable(node->value, block_size)) {
            node = node->next;
        }

        if (node != NULL) {
            entry = node->value;
        }
    }

    /*
     * Our best case is that we find a node in the index that can fit the size.
     */
    if (entry != NULL) {
        drop_sitter(heap, entry);
        rkmalloc_index_entry* index = (rkmalloc_index_entry*) ((uintptr_t) entry - sizeof(list_node_t));
        entry->free = false;
        entry->used_size = size;
        heap->total_allocated_blocks_size += entry->block_size;
        heap->total_allocated_used_size += size;
        void* ptr = &index->ptr;

        spin_unlock(&heap->lock);
        return ptr;
    }

    /* TODO(kaendfinger): Implement combining blocks. */

    size_t header_and_size = sizeof(rkmalloc_index_entry) + block_size;

    rkmalloc_index_entry* blk = heap->expand(header_and_size);

    if (blk == NULL) {
        spin_unlock(&heap->lock);
        return NULL;
    }
    
    list_init_node(&blk->node);
    
    blk->node.list = &heap->index;

    entry = &blk->entry;

    entry->free = false;
    entry->block_size = block_size;
    entry->used_size = size;
    
#ifndef RKMALLOC_DISABLE_MAGIC
    entry->magic = rkmagic((uintptr_t) &blk->ptr);
#endif

    heap->total_allocated_blocks_size += block_size;
    heap->total_allocated_used_size += size;

    blk->node.value = entry;

    list_insert_node_before(heap->index.head, &blk->node);

    void* ptr = &blk->ptr;
    spin_unlock(&heap->lock);
    return ptr;
}

void* rkmalloc_resize(rkmalloc_heap* heap, void* ptr, size_t new_size) {
    rkmalloc_entry* entry = NULL;
    list_node_t* node = get_pointer_entry(heap, ptr, &entry);
    if (node == NULL) {
        printf(
            WARN "Failed to resize 0x%x: "
                "we don't have that in our heap!\n",
            ptr
        );
        return NULL;
    }

    if (entry->free) {
        printf(
            WARN "Failed to resize 0x%x: "
                "heap says it is free!\n",
            ptr
        );
        return NULL;
    }

    if (entry->used_size == new_size) {
        return ptr;
    }

    if (entry->block_size <= new_size &&
        entry->used_size <= new_size) {
        entry->used_size = new_size;
        return ptr;
    }

    void* out = rkmalloc_allocate(heap, new_size);
    if (entry->used_size < new_size) {
        memset(
            (void*) ((uintptr_t) out + entry->used_size),
            0,
            new_size - entry->used_size
        );
    }
    memcpy(out, ptr, entry->used_size);
    rkmalloc_free(heap, ptr);
    return out;
}

void rkmalloc_free(rkmalloc_heap* heap, void* ptr) {
    if (ptr == NULL) {
        return;
    }

    rkmalloc_entry* entry = NULL;
    list_node_t* node = get_pointer_entry(heap, ptr, &entry);

    if (node == NULL) {
        printf(WARN "Attempted to free an invalid pointer (0x%x)\n", ptr);
    } else if (entry->free) {
        printf(WARN "Attempted to free an already freed pointer (0x%x)\n", ptr);
    } else {
        entry->free = true;
        insert_sitter(heap, entry);
        heap->total_allocated_blocks_size -= entry->block_size;
        heap->total_allocated_used_size -= entry->used_size;
    }
}
