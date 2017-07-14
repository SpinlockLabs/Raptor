#include "ata.h"

#include <kernel/arch/x86/devices/pci/pci.h>
#include <kernel/arch/x86/irq.h>
#include <kernel/arch/x86/io.h>
#include <liblox/printf.h>
#include <kernel/disk/disk.h>
#include <kernel/arch/x86/heap.h>

static char ata_drive_char = 'a';
static int cdrom_number = 0;
static uint32_t ata_pci = 0x00000000;

static ata_device_t ata_primary_master = {.io_base = 0x1F0, .control = 0x3F6, .slave = 0};
static ata_device_t ata_primary_slave = {.io_base = 0x1F0, .control = 0x3F6, .slave = 1};
static ata_device_t ata_secondary_master = {.io_base = 0x170, .control = 0x376, .slave = 0};
static ata_device_t ata_secondary_slave = {.io_base = 0x170, .control = 0x376, .slave = 1};

static uint8_t ata_inb(uint16_t port, uint16_t offset) {
    return inb(port + offset);
}

static void ata_outb(uint16_t port, uint16_t offset, uint8_t value) {
    outb(port + offset, value);
}

static uint32_t read_ata(block_device_t* blk, uint32_t offset, uint32_t size, uint8_t* buffer) {
    // TODO
    return 0;
}

static block_device_error_t block_stat(block_device_t* dev, block_device_stat_t* stat) {
    stat->size = (size_t) ((ata_device_t*)dev->private.owner)->identity.sectors_48;
    return BLOCK_DEVICE_ERROR_OK;
}

static void ata_io_wait(ata_device_t* dev) {
    ata_inb(dev->io_base, ATA_REG_ALTSTATUS);
    ata_inb(dev->io_base, ATA_REG_ALTSTATUS);
    ata_inb(dev->io_base, ATA_REG_ALTSTATUS);
    ata_inb(dev->io_base, ATA_REG_ALTSTATUS);
}

static int ata_status_wait(ata_device_t* dev, int timeout) {
    int status = 0;
    if (timeout > 0) {
        int i = 0;
        while ((status == ata_inb(dev->io_base, ATA_REG_STATUS)) & ATA_SR_BSY && (i < timeout)) i++;
    } else {
        while ((status == ata_inb(dev->io_base, ATA_REG_STATUS)) & ATA_SR_BSY);
    }
    return status;
}

static int ata_wait(ata_device_t* dev, int advanced) {
    uint8_t status = 0;
    ata_io_wait(dev);
    status = ata_status_wait(dev, -1);

    if (advanced) {
        status = ata_inb(dev->io_base, ATA_REG_STATUS);
        if (status & ATA_SR_ERR) return 1;
        if (status & ATA_SR_DF) return 1;
        if (!(status & ATA_SR_DRQ)) return 1;
    }

    return 0;
}

static void ata_soft_reset(ata_device_t* dev) {
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

static block_device_t* ata_device_create(ata_device_t* device) {
    char name[64];
    sprintf(name, "atadev%d", ata_drive_char - 'a');
    block_device_t* block_dev = block_device_create(name);
    block_dev->private.owner = device;
    block_dev->ops.stat = block_stat;
    block_device_register(block_dev);
    return block_dev;
}

static void ata_device_init(ata_device_t* dev) {
    printf(DEBUG "Initializing IDE device on bus %d\n", dev->io_base);

    ata_outb(dev->io_base, 1, 1);
    outb(dev->control, 0);

    ata_outb(dev->io_base, ATA_REG_HDDEVSEL, 0xA0 | dev->slave << 4);
    ata_io_wait(dev);

    int status = ata_inb(dev->io_base, ATA_REG_COMMAND);
    printf(DEBUG "Device status: %d\n", status);

    ata_wait(dev, 0);

    uint16_t* buf = (uint16_t*)&dev->identity;

    for (int i = 0; i < 256; ++i) {
        buf[i] = ins(dev->io_base);
    }

    uint8_t* ptr = (uint8_t*)&dev->identity.model;
    for (int i = 0; i < 39; i += 2) {
        uint8_t tmp = ptr[i + 1];
        ptr[i + 1] = ptr[i];
        ptr[i] = tmp;
    }

    dev->is_atapi = 0;

    printf(DEBUG "Device name: %s\n", dev->identity.model);
    printf(DEBUG "Sectors (48): %d\n", (uint32_t)dev->identity.sectors_48);
    printf(DEBUG "Sectors (28): %d\n", (uint32_t)dev->identity.sectors_28);

    printf(DEBUG "Setting up DMA...\n");
    dev->dma_prdt = (void*) kpmalloc_p(sizeof(prdt_t) * 1, &dev->dma_prdt_phys);
    dev->dma_start = (void *)kpmalloc_p(4096, &dev->dma_start_phys);

    printf(DEBUG "Putting prdt    at 0x%x (0x%x phys)\n", dev->dma_prdt, dev->dma_prdt_phys);
    printf(DEBUG "Putting prdt[0] at 0x%x (0x%x phys)\n", dev->dma_start, dev->dma_start_phys);

    dev->dma_prdt[0].offset = dev->dma_start_phys;
    dev->dma_prdt[0].bytes = 512;
    dev->dma_prdt[0].last = 0x8000;

    printf(DEBUG "ATA PCI device ID: 0x%x\n", ata_pci);

    uint16_t command_reg = pci_read_field(ata_pci, PCI_COMMAND, 4);
    printf(DEBUG "COMMAND register before: 0x%4x", command_reg);
    if (command_reg & (1 << 2)) {
        printf(DEBUG "Bus mastering already enabled.\n");
    } else {
        command_reg |= (1 << 2);
        printf(DEBUG "Enabling bus mastering...\n");
        pci_write_field(ata_pci, PCI_COMMAND, 4, command_reg);
        command_reg = pci_read_field(ata_pci, PCI_COMMAND, 4);
        printf(DEBUG "COMMAND register after: 0x%4x\n", command_reg);
    }

    dev->bar4 = pci_read_field(ata_pci, PCI_BAR4, 4);
    printf(DEBUG "BAR4: 0x%x\n", dev->bar4);

    if (dev->bar4 & 0x00000001) {
        dev->bar4 = dev->bar4 & 0xFFFFFFFC;
    } else {
        printf(WARN "ATA Bus master registers are usually I/O ports.\n");
        return;
    }
}

static int ata_device_detect(ata_device_t* dev) {
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
        char devname[64];
        sprintf((char*)&devname, "/dev/hd%c", ata_drive_char);
        block_device_t* device = ata_device_create(dev);
        ata_drive_char++;
        ata_device_init(dev);

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
