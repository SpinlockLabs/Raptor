#include "rtl8139.h"

#include <kernel/arch/x86/io.h>
#include <kernel/arch/x86/devices/pci/pci.h>
#include <kernel/arch/x86/heap.h>
#include <liblox/io.h>

static void rtl8139_init(uint32_t device) {
    uint32_t rtl_bar0 = pci_read_field(device, PCI_BAR0, 4);
    uint32_t rtl_bar1 = pci_read_field(device, PCI_BAR1, 4);
    uint32_t rtl_iobase = 0x00000000;

    if (rtl_bar0 & 0x00000001) {
        rtl_iobase = rtl_bar0 & 0xFFFFFFFC;
    }

    uint16_t command_reg = (uint16_t) pci_read_field(device, PCI_COMMAND, 2);
    command_reg |= (1 << 2);
    pci_write_field(device, PCI_COMMAND, 2, command_reg);

    uint8_t mac[6];
    for (int i = 0; i < 6; ++i) {
        mac[i] = ins(rtl_iobase + i);
    }

    printf(DEBUG "%2x:%2x:%2x:%2x:%2x:%2x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Turn on
    outb((uint16_t) (rtl_iobase + 0x52), 0x0);

    // Software reset
    outb(rtl_iobase + 0x37, 0x10);
    while ((inb(rtl_iobase + 0x37) & 0x10) != 0) {}

    //uintptr_t rx_buffer = kpmalloc_a(8192 + 16);
    //outs((uint16_t) (rtl_iobase + 0x30), (uint16_t) rx_buffer);
}

static void find_rtl8139(uint32_t device, uint16_t vendor_id, uint16_t device_id, void* extra) {
    unused(extra);

    if ((vendor_id == 0x10EC) && (device_id == 0x8139)) {
        rtl8139_init(device);
    }
}

void rtl8139_setup(void) {
    pci_scan(&find_rtl8139, -1, NULL);
}
