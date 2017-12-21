#include "rkmalloc.h"

#include <liblox/hex.h>

#ifndef RKMALLOC_DISABLE_MAGIC
uintptr_t rkmagic(uintptr_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}
#endif

rkmalloc_error rkmalloc_init_heap(rkmalloc_heap* heap) {
    if (heap->grab_block == NULL) {
        return RKMALLOC_ERROR_INVALID_POINTER;
    }

    heap->total_allocated_blocks_size = 0;
    heap->total_allocated_used_size = 0;

    spin_init(&heap->lock);
    SET_SPIN_LOCK_LABEL(&heap->lock, "rkmalloc heap");

    list_init(&heap->index);
    heap->index.free_values = false;

    for (uint i = 0; i < RKMALLOC_SITTER_COUNT; i++) {
        heap->sitters[i] = NULL;
    }

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

static rkmalloc_index_entry* reserve_block(
    rkmalloc_heap* heap,
    size_t size
) {
    size_t header_and_size = sizeof(rkmalloc_index_entry) + size;
    rkmalloc_index_entry* blk = heap->grab_block(header_and_size);

    if (blk == NULL) {
        return NULL;
    }

    list_init_node(&blk->node);

    blk->node.list = &heap->index;

    rkmalloc_entry* entry = &blk->entry;
    entry->free = true;
    entry->sitting = false;
    entry->block_size = size;
    entry->used_size = 0;

#ifndef RKMALLOC_DISABLE_MAGIC
    entry->magic = rkmagic((uintptr_t) &blk->ptr);
#endif

    heap->total_allocated_blocks_size += size;

    blk->node.value = entry;
    list_insert_node_before(heap->index.head, &blk->node);

    return blk;
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

static size_t get_block_size(size_t size) {
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    size++;

    if (size < 8) {
        size = 8;
    }

    return size;
}

static bool is_block_usable(rkmalloc_entry* entry, size_t block_size) {
    if (!entry->free) {
        return false;
    }

    return block_size == entry->block_size;
}

void* rkmalloc_allocate_align(rkmalloc_heap* heap, size_t size, size_t align) {
    size_t mask = align - 1;
    size_t true_size = size + align;

    if ((get_block_size(size) - size) >= align) {
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

    size_t block_size = get_block_size(size);

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

    if (entry != NULL) {
        drop_sitter(heap, entry);
        rkmalloc_index_entry* index = (rkmalloc_index_entry*) (
            (uintptr_t) entry - sizeof(list_node_t)
        );
        entry->free = false;
        entry->used_size = size;
        heap->total_allocated_blocks_size += entry->block_size;
        heap->total_allocated_used_size += size;
        void* ptr = &index->ptr;

        spin_unlock(&heap->lock);
        return ptr;
    }

    rkmalloc_index_entry* blk = reserve_block(heap, block_size);

    if (blk == NULL) {
        spin_unlock(&heap->lock);
        return NULL;
    }

    blk->entry.free = false;
    heap->total_allocated_used_size += size;

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

void rkmalloc_reserve(rkmalloc_heap* heap, size_t size) {
    spin_lock(&heap->lock);
    reserve_block(heap, size);
    spin_unlock(&heap->lock);
}

uint rkmalloc_reduce(
    rkmalloc_heap* heap,
    bool aggressive
) {
    spin_lock(&heap->lock);

    if (heap->return_block == NULL) {
        spin_unlock(&heap->lock);
        return 0;
    }

    uint count = 0;

    list_node_t* current = heap->index.head;
    while (current != NULL) {
        rkmalloc_entry* entry = current->value;
        if (!entry->free) {
            current = current->next;
            continue;
        }

        if (!aggressive && entry->sitting) {
            current = current->next;
            continue;
        }

        list_node_t* prev = current->prev;
        list_remove(current);
        heap->return_block(current);
        count++;

        if (prev == NULL) {
            current = NULL;
        } else {
            current = prev->next;
        }
    }
    spin_unlock(&heap->lock);
    return count;
}
