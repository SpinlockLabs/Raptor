#include "console.h"

#include <kernel/rkmalloc/rkmalloc.h>
#include <kernel/heap.h>

static void debug_kheap_used(tty_t* tty, const char* input) {
    unused(input);

    rkmalloc_heap* heap = heap_get();
    tty_printf(tty, "Object Allocation: %d bytes\n", heap->total_allocated_used_size);
    tty_printf(tty, "Block Allocation: %d bytes\n", heap->total_allocated_blocks_size);
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
    debug_console_register_command("kheap-used", debug_kheap_used);
}
