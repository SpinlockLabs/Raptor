#include <liblox/io.h>
#include <liblox/hex.h>

#include <kernel/devices/pci/pci.h>

#include "pci_init.h"

static void found_pci_device(uint32_t device, uint16_t vendorid, uint16_t deviceid, void *extra) {
  unused(extra);

  const char *vendor = pci_vendor_lookup(vendorid);
  const char *dev = pci_device_lookup(vendorid, deviceid);

  puts(DEBUG "Found PCI Device: Vendor ID = ");
  putint_hex((int) vendorid);
  puts(", Device ID = ");
  putint_hex((int) deviceid);
  puts(", Device Location: ");
  putint_hex((int) device);

  if (vendor != NULL) {
    puts(", Vendor: ");
    puts((char*) vendor);
  }

  if (dev != NULL) {
    puts(", Device: ");
    puts((char*) dev);
  }

  puts("\n");
}

void pci_init(void) {
  pci_scan(found_pci_device, -1, NULL);
}
