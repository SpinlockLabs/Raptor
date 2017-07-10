#include "debug.h"
#include "heap.h"

#include <kernel/tty.h>
#include <kernel/network/iface.h>

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

void debug_crash(tty_t* tty, const char* input) {
    unused(input);
    unused(tty);

    memcpy(NULL, NULL, 1);
}

void debug_pcnet_mac(tty_t* tty, const char* input) {
    unused(input);

    network_iface_t* iface = network_iface_get("pcnet");

    if (iface == NULL) {
        tty_printf(tty, "PCNET is not configured.\n");
        return;
    }

    uint8_t mac[6] = {0};
    if (network_iface_get_mac(iface, mac) != IFACE_ERR_OK) {
        tty_printf(tty, "Network interface error.\n");
        return;
    }

    tty_printf(tty,
               "%2x:%2x:%2x:%2x:%2x:%2x\n",
               mac[0],
               mac[1],
               mac[2],
               mac[3],
               mac[4],
               mac[5]
    );
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
    debug_console_register_command("pcnet-mac", debug_pcnet_mac);
    debug_console_register_command("crash", debug_crash);
}
