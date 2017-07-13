#include "ata.h"

#include <kernel/arch/x86/devices/pci/pci.h>
#include <kernel/arch/x86/irq.h>
#include <kernel/arch/x86/io.h>

static struct ata_device ata_primary_master = {.io_base = 0x1F0, .control = 0x3F6, .slave = 0};
static struct ata_device ata_primary_slave = {.io_base = 0x1F0, .control = 0x3F6, .slave = 1};
static struct ata_device ata_secondary_master = {.io_base = 0x170, .control = 0x376, .slave = 0};
static struct ata_device ata_secondary_slave = {.io_base = 0x170, .control = 0x376, .slave = 1};

static uint8_t ata_inb(uint16_t port, uint16_t offset) {
    return inb(port + offset);
}

static void ata_outb(uint16_t port, uint16_t offset, uint8_t value) {
    outb(port + offset, value);
}

static int ata_status_wait(struct ata_device* dev, int timeout) {
    int status = 0;
    if (timeout > 0) {
        int i = 0;
        while ((status == ata_inb(dev->io_base, ATA_REG_STATUS)) & ATA_SR_BSY && (i < timeout)) i++;
    } else {
        while ((status == ata_inb(dev->io_base, ATA_REG_STATUS)) & ATA_SR_BSY);
    }
    return status;
}

static void ata_io_wait(struct ata_device* dev) {
    ata_inb(dev->io_base, ATA_REG_ALTSTATUS);
    ata_inb(dev->io_base, ATA_REG_ALTSTATUS);
    ata_inb(dev->io_base, ATA_REG_ALTSTATUS);
    ata_inb(dev->io_base, ATA_REG_ALTSTATUS);
}

static void ata_soft_reset(struct ata_device* dev) {
    outb(dev->control, 0x04);
    ata_io_wait(dev);
    outb(dev->control, 0x00);
}

static int ata_irq_handler(struct regs* r) {
    unused(r);
    return 1;
}

static int ata_irq_handler_s(struct regs* r) {
    unused(r);
    return 1;
}

static int ata_device_detect(struct ata_device* dev) {
    ata_soft_reset(dev);
    ata_io_wait(dev);
    ata_outb(dev->io_base, ATA_REG_HDDEVSEL, 0xA0 | dev->slave << 4);
    ata_io_wait(dev);
    ata_status_wait(dev, 10000);

    unsigned char cl = ata_inb(dev->io_base, ATA_REG_LBA1);
    unsigned char ch = ata_inb(dev->io_base, ATA_REG_LBA2);

    if (cl == 0xFF && ch == 0xFF) {
        return 0;
    }

    if ((cl == 0x00 && ch == 0x00) ||
        (cl == 0x3C && ch == 0xC3)) {
        puts(DEBUG "Hard drive detected\n");
        return 1;
    } else if ((cl == 0x14 && ch == 0xEB) ||
               (cl == 0x69 && ch == 0x96)) {
        puts(DEBUG "CD-ROM detected\n");
        return 2;
    }

    return 0;
}

static void find_ata(uint32_t device, uint16_t vid, uint16_t did,
                     void *extra) {
    unused(device);
    unused(extra);

    if ((vid == 0x8086) &&
        (did == 0x7010)) {
        // Match with a IDE controller.
    }
}

void ata_setup(void) {
    pci_scan(&find_ata, -1, NULL);

    irq_add_handler(14, ata_irq_handler);
    irq_add_handler(15, ata_irq_handler_s);

    ata_device_detect(&ata_primary_master);
    ata_device_detect(&ata_primary_slave);
    ata_device_detect(&ata_secondary_master);
    ata_device_detect(&ata_secondary_slave);
}
