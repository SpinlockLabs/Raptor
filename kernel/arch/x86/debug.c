#include "debug.h"
#include "heap.h"

#include <liblox/io.h>

#include <kernel/tty.h>
#include <kernel/debug/console.h>

#include <kernel/arch/x86/devices/pci/pci.h>
#include <kernel/rkmalloc/rkmalloc.h>

void debug_kpused(tty_t* tty, const char* input) {
    unused(input);

    size_t size = kpused();
    int kb = size / 1024;

    tty_printf(tty, "Used: %d bytes, %d kb\n", size, kb);
}

extern rkmalloc_heap* kheap;

void debug_kheap_used(tty_t* tty, const char* input) {
    unused(input);

    tty_printf(tty, "Object Allocation: %d bytes\n", kheap->total_allocated_used_size);
    tty_printf(tty, "Block Allocation: %d bytes\n", kheap->total_allocated_blocks_size);
}

static void pci_show_simple(uint32_t loc, uint16_t vid, uint16_t did, void* extra) {
    tty_t* tty = extra;

    const char* vendor = pci_vendor_lookup(vid);
    const char* dev = pci_device_lookup(vid, did);

    if (vendor == NULL) {
        vendor = "Unknown";
    }

    if (dev == NULL) {
        dev = "Unknown";
    }

    tty_printf(tty,
               "(0x%x) [%s] (0x%x) by [%s] (0x%x)\n",
               loc, dev, did, vendor, vid);
}

void debug_pci_list(tty_t* tty, const char* input) {
    unused(input);

    pci_scan(pci_show_simple, -1, tty);
}

void debug_x86_init(void) {
    debug_console_register_command("kpused", debug_kpused);
    debug_console_register_command("pci-list", debug_pci_list);
    debug_console_register_command("kheap-used", debug_kheap_used);
}
