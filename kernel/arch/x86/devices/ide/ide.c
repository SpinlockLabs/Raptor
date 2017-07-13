#include "ide.h"

#include <kernel/arch/x86/devices/pci/pci.h>

static void find_ide(uint32_t device, uint16_t vid, uint16_t did,
                     void *extra) {
    unused(device);
    unused(extra);

    if ((vid == 0x8086) &&
        (did == 0x7010)) {
        // Match with a IDE controller.
    }
}

void ide_setup(void) {
    pci_scan(&find_ide, -1, NULL);
}
