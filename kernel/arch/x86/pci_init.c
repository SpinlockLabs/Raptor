#include <liblox/io.h>

#include <kernel/arch/x86/devices/pci/pci.h>
#include <kernel/device/registry.h>
#include <liblox/printf.h>

#include "pci_init.h"

static int current_id = 0;

static void found_pci_device(uint32_t loc, uint16_t vid, uint16_t did, void* extra) {
    unused(extra);
    unused(loc);

    const char* vendor = pci_vendor_lookup(vid);
    const char* dev = pci_device_lookup(vid, did);

    printf(DEBUG "Found PCI Device: ");
    if (*vendor) {
        printf("Vendor: %s", vendor);
    } else {
        printf("Vendor ID: %x", vid);
    }

    if (*dev) {
        printf(", Device: %s", dev);
    } else {
        printf(", Device ID: %x", did);
    }

    printf("\n");

    device_entry_t* root = device_root();
    pci_device_t* device = zalloc(sizeof(pci_device_t));

    sprintf(device->name, "pci%d", ++current_id);
    device->address = loc;
    device->device_id = did;
    device->vendor_id = vid;

    device_register(
        root,
        device->name,
        DEVICE_CLASS_PCI,
        device
    );
}

void pci_init(void) {
    pci_scan(found_pci_device, -1, NULL);
}
