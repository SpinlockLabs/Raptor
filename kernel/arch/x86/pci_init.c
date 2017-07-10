#include <liblox/io.h>

#include <kernel/arch/x86/devices/pci/pci.h>

#include "pci_init.h"

static void found_pci_device(uint32_t loc, uint16_t vid, uint16_t did, void* extra) {
    unused(extra);
    unused(loc);

    const char* vendor = pci_vendor_lookup(vid);
    const char* dev = pci_device_lookup(vid, did);

    printf(DEBUG "Found PCI Device: ");
    if (vendor != NULL) {
        printf("Vendor: %s", vendor);
    } else {
        printf("Vendor ID: %x", vid);
    }

    if (dev != NULL) {
        printf(", Device: %s", dev);
    } else {
        printf(", Device ID: %x", did);
    }

    printf("\n");
}

void pci_init(void) {
    pci_scan(found_pci_device, -1, NULL);
}
