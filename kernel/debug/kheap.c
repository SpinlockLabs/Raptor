#include "console.h"

#include <kernel/rkmalloc/rkmalloc.h>
#include <kernel/heap.h>

static void debug_kheap_stats(tty_t* tty, const char* input) {
    unused(input);

    rkmalloc_heap* heap = heap_get();
    spin_lock(&heap->lock);

    tty_printf(tty, "Used Object Allocations: %d bytes\n", heap->total_allocated_used_size);
    tty_printf(tty, "Used Block Allocations: %d bytes\n", heap->total_allocated_blocks_size);

    size_t meta_total = 0;
    size_t full_total = 0;
    size_t reclaimable_entries = 0;
    size_t reclaimable_block_total = 0;

    list_for_each(node, &heap->index) {
        rkmalloc_entry* entry = node->value;
        meta_total += sizeof(rkmalloc_entry);
        full_total += sizeof(rkmalloc_entry) + entry->block_size;

        if (entry->free) {
            reclaimable_entries++;
            reclaimable_block_total += entry->block_size;
        }
    }

    spin_unlock(&heap->lock);

    tty_printf(tty, "Reclaimable Entries: %d\n", reclaimable_entries);
    tty_printf(tty, "Reclaimable Block Allocations: %d bytes\n", reclaimable_block_total);
    tty_printf(tty, "Metadata Usage: %d bytes\n", meta_total);
    tty_printf(tty, "Total Usage: %d bytes\n", full_total);
}

static void debug_kheap_dump(tty_t* tty, const char* input) {
    unused(input);

    rkmalloc_heap* kheap = heap_get();
    list_t* list = &kheap->index;

    size_t index = 0;
    list_for_each(node, list) {
        rkmalloc_entry* entry = node->value;
        tty_printf(tty,
                   "%d[block = %d bytes, used = %d bytes, location = 0x%x, status = %s]\n",
                   index,
                   entry->block_size,
                   entry->used_size,
                   entry->ptr,
                   entry->free ? "free" : "used"
        );
        index++;
    }
}

void debug_kheap_init(void) {
    debug_console_register_command("kheap-dump", debug_kheap_dump);
    debug_console_register_command("kheap-stats", debug_kheap_stats);
}
