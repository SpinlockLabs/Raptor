#include <liblox/io.h>

#include <kernel/arch/x86/devices/pci/pci.h>

#include "pci_init.h"

static void found_pci_device(uint32_t device, uint16_t vendorid, uint16_t deviceid, void *extra) {
  unused(extra);
  unused(device);

  const char *vendor = pci_vendor_lookup(vendorid);
  const char *dev = pci_device_lookup(vendorid, deviceid);

  printf(DEBUG "Found PCI Device: ");
  if (vendor != NULL) {
    printf("Vendor: %s", vendor);
  } else {
    printf("Vendor ID: %x", vendorid);
  }

  if (dev != NULL) {
    printf(", Device: %s", dev);
  } else {
    printf(", Device ID: %x", deviceid);
  }

  printf("\n");
}

void pci_init(void) {
  pci_scan(found_pci_device, -1, NULL);
}
