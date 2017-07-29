#include "ata.h"

#include <liblox/string.h>
#include <liblox/printf.h>

#include <kernel/arch/x86/devices/pci/pci.h>

#include <kernel/arch/x86/irq.h>
#include <kernel/arch/x86/io.h>
#include <kernel/arch/x86/heap.h>

#include <kernel/disk/block.h>

#include <kernel/spin.h>

#define dbg(msg, ...) printf(DEBUG "[ATA] " msg, ##__VA_ARGS__)
#define info(msg, ...) printf(INFO "[ATA] " msg, ##__VA_ARGS__)

static char ata_drive_char = 'a';
static uint32_t ata_pci = 0x00000000;

static spin_lock_t ata_lock;

static ata_device_t ata_primary_master = {
    .io_base = 0x1F0,
    .control = 0x3F6,
    .slave = 0
};

static ata_device_t ata_primary_slave = {
    .io_base = 0x1F0,
    .control = 0x3F6,
    .slave = 1
};

static ata_device_t ata_secondary_master = {
    .io_base = 0x170,
    .control = 0x376,
    .slave = 0
};

static ata_device_t ata_secondary_slave = {
    .io_base = 0x170,
    .control = 0x376,
    .slave = 1
};

static uint8_t ata_inb(uint16_t port, uint16_t offset) {
    return inb(port + offset);
}

static void ata_outb(uint16_t port, uint16_t offset, uint8_t value) {
    outb(port + offset, value);
}

static block_device_error_t ata_block_stat(
    block_device_t* dev,
    block_device_stat_t* stat) {
    ata_device_t* ata = dev->internal.owner;
    stat->size = (size_t) (ata->identity.sectors_48 * ATA_SECTOR_SIZE);
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
        while ((status == ata_inb(dev->io_base, ATA_REG_STATUS)) & ATA_SR_BSY && (i < timeout)) {
            i++;
        }
    } else {
        while ((status == ata_inb(dev->io_base, ATA_REG_STATUS)) & ATA_SR_BSY) {
        }
    }
    return status;
}

static int ata_wait(ata_device_t* dev, int advanced) {
    uint8_t status = 0;
    ata_io_wait(dev);
    ata_status_wait(dev, -1);

    if (advanced) {
        status = ata_inb(dev->io_base, ATA_REG_STATUS);
        if (status & ATA_SR_ERR) {
            return 1;
        }

        if (status & ATA_SR_DF) {
            return 1;
        }

        if (!(status & ATA_SR_DRQ)) {
            return 1;
        }
    }

    return 0;
}

static void ata_soft_reset(ata_device_t* dev) {
    outb(dev->control, 0x04);
    ata_io_wait(dev);
    outb(dev->control, 0x00);
}

static int ata_irq_handler(cpu_registers_t* r) {
    unused(r);
    ata_inb(ata_primary_master.io_base, ATA_REG_STATUS);
    return 1;
}

static int ata_irq_handler_s(cpu_registers_t* r) {
    unused(r);
    ata_inb(ata_secondary_master.io_base, ATA_REG_STATUS);
    return 1;
}

static size_t ata_max_offset(ata_device_t* dev) {
    uint64_t sectors = dev->identity.sectors_48;
    if (!sectors) {
        sectors = dev->identity.sectors_28;
    }
    return (size_t) (sectors * ATA_SECTOR_SIZE);
}

static block_device_error_t ata_block_read_sector(
    block_device_t* blk,
    size_t sector,
    uint8_t* buffer) {
    ata_device_t* ata = blk->internal.owner;

    uint16_t bus = ata->io_base;
    uint8_t slave = (uint8_t) ata->slave;

    spin_lock(&ata_lock);

    ata_wait(ata, 0);
    outb(ata->bar4, 0x00);
    outl((uint16_t) (ata->bar4 + 0x04), ata->dma_prdt_phys);
    outb((uint16_t) (ata->bar4 + 0x2), (uint8_t) (inb((uint16_t) (ata->bar4 + 0x02)) | 0x04 | 0x02));
    outb(ata->bar4, 0x08);

    int_enable();
    while (true) {
        uint8_t status = ata_inb(bus, ATA_REG_STATUS);
        if (!(status & ATA_SR_BSY)) {
            break;
        }
    }
    ata_outb(bus, ATA_REG_CONTROL, 0x00);
    ata_outb(bus, ATA_REG_HDDEVSEL,
             (uint8_t) (0xe0 | slave << 4 | (sector & 0x0f000000) >> 24));
    ata_io_wait(ata);

    ata_outb(bus, ATA_REG_FEATURES, 0x00);
    ata_outb(bus, ATA_REG_SECCOUNT0, 1);
    ata_outb(bus, ATA_REG_LBA0, (uint8_t) ((sector & 0x000000ff) >> 0));
    ata_outb(bus, ATA_REG_LBA1, (uint8_t) ((sector & 0x0000ff00) >> 8));
    ata_outb(bus, ATA_REG_LBA2, (uint8_t) ((sector & 0x00ff0000) >> 16));

    while (true) {
        uint8_t status = ata_inb(bus, ATA_REG_STATUS);
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRDY)) {
            break;
        }
    }

    ata_outb(bus, ATA_REG_COMMAND, ATA_CMD_READ_DMA);
    ata_io_wait(ata);

    ata_outb(ata->bar4, 0, 0x08 | 0x01);

    while (true) {
        uint8_t status = ata_inb(ata->bar4, 0x02);
        uint8_t dstatus = ata_inb(bus, ATA_REG_STATUS);

        if (!(status & 0x04)) {
            continue;
        }

        if (!(dstatus & ATA_SR_BSY)) {
            break;
        }
    }
    int_disable();

    memcpy(buffer, ata->dma_start, 512);
    ata_outb(ata->bar4, 0x2, (uint8_t) (inb((uint16_t) (ata->bar4 + 0x02)) | 0x04 | 0x02));

    spin_unlock(&ata_lock);

    return BLOCK_DEVICE_ERROR_OK;
}

static block_device_error_t ata_block_read(
    block_device_t* blk,
    size_t offset,
    uint8_t* buffer,
    size_t size) {
    block_device_error_t error = BLOCK_DEVICE_ERROR_OK;
    ata_device_t* ata = blk->internal.owner;
    size_t start_block = offset / ATA_SECTOR_SIZE;
    size_t end_block = (offset + size - 1) / ATA_SECTOR_SIZE;

    size_t x_offset = 0;
    size_t max_offset = ata_max_offset(ata);

    if (offset > max_offset) {
        return BLOCK_DEVICE_ERROR_BAD_OFFSET;
    }

    if (offset > max_offset) {
        size = max_offset - offset;
    }

    uint8_t* tmp = malloc(ATA_SECTOR_SIZE);

    if (offset % ATA_SECTOR_SIZE) {
        size_t prefix_size = (ATA_SECTOR_SIZE - (offset % ATA_SECTOR_SIZE));
        error = ata_block_read_sector(blk, start_block, tmp);
        if (error != BLOCK_DEVICE_ERROR_OK) {
            free(tmp);
            return error;
        }

        memcpy(
            buffer,
            (void*) ((uintptr_t) tmp + (offset % ATA_SECTOR_SIZE)),
            prefix_size
        );
        x_offset += prefix_size;
        start_block++;
    }

    if ((offset + size) % ATA_SECTOR_SIZE && start_block <= end_block) {
        size_t postfix_size = (offset + size) % ATA_SECTOR_SIZE;
        error = ata_block_read_sector(blk, end_block, tmp);
        if (error != BLOCK_DEVICE_ERROR_OK) {
            free(tmp);
            return error;
        }

        memcpy(
            (uint8_t*) ((uintptr_t) buffer + size - postfix_size),
            tmp,
            postfix_size
        );

        end_block--;
    }

    while (start_block <= end_block) {
        error = ata_block_read_sector(
            blk,
            start_block,
            (uint8_t*) ((uintptr_t)  buffer + x_offset)
        );

        if (error != BLOCK_DEVICE_ERROR_OK) {
            free(tmp);
            return error;
        }

        x_offset += ATA_SECTOR_SIZE;
        start_block++;
    }

    free(tmp);

    return error;
}

static block_device_error_t ata_block_write(
    block_device_t* blk,
    size_t offset,
    uint8_t* buffer,
    size_t size) {
    unused(blk);
    unused(offset);
    unused(buffer);
    unused(size);
    return BLOCK_DEVICE_ERROR_UNHANDLED;
}

static block_device_t* ata_device_create(ata_device_t* device) {
    char name[64];
    sprintf(name, "ata%d", ata_drive_char - 'a');
    block_device_t* block_dev = block_device_create(name);
    block_dev->internal.owner = device;
    block_dev->ops.stat = ata_block_stat;
    block_dev->ops.read = ata_block_read;
    block_dev->ops.write = ata_block_write;
    return block_dev;
}

static void ata_device_init(ata_device_t* dev) {
    ata_outb(dev->io_base, 1, 1);
    outb(dev->control, 0);

    ata_outb(dev->io_base, ATA_REG_HDDEVSEL, (uint8_t) (0xA0 | dev->slave << 4));
    ata_io_wait(dev);

    ata_outb(dev->io_base, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ata_io_wait(dev);

    ata_inb(dev->io_base, ATA_REG_COMMAND);

    ata_wait(dev, 0);

    uint16_t* buf = (uint16_t*) &dev->identity;

    for (int i = 0; i < 256; ++i) {
        buf[i] = ins(dev->io_base);
    }

    uint8_t* ptr = (uint8_t*) &dev->identity.model;
    for (int i = 0; i < 39; i += 2) {
        uint8_t tmp = ptr[i + 1];
        ptr[i + 1] = ptr[i];
        ptr[i] = tmp;
    }

    ata_wait(dev, 1);

    dev->is_atapi = 0;

    dev->dma_prdt = (void*) kpmalloc_ap(sizeof(prdt_t) * 1, &dev->dma_prdt_phys);
    dev->dma_start = (void*) kpmalloc_ap(4096, &dev->dma_start_phys);

    memset(dev->dma_prdt, 0, sizeof(prdt_t) * 1);
    memset(dev->dma_start, 0, 4096);

    dev->dma_prdt[0].offset = dev->dma_start_phys;
    dev->dma_prdt[0].bytes = 512;
    dev->dma_prdt[0].last = 0x8000;

    uint16_t command_reg = (uint16_t) pci_read_field(ata_pci, PCI_COMMAND, 4);
    if (command_reg & (1 << 2)) {
        dbg("Bus mastering already enabled.\n");
    } else {
        command_reg |= (1 << 2);
        pci_write_field(ata_pci, PCI_COMMAND, 4, command_reg);
    }

    uint32_t bar4 = pci_read_field(ata_pci, PCI_BAR4, 4);

    if (bar4 & 0x00000001) {
        dev->bar4 = (uint16_t) (bar4 & 0xFFFFFFFC);
    } else {
        dbg("Bus master registers are usually I/O ports.\n");
        return;
    }
}

static int ata_device_detect(ata_device_t* dev) {
    ata_soft_reset(dev);
    ata_io_wait(dev);
    ata_outb(dev->io_base, ATA_REG_HDDEVSEL, (uint8_t) (0xA0 | dev->slave << 4));
    ata_io_wait(dev);
    ata_status_wait(dev, 10000);

    unsigned char cl = ata_inb(dev->io_base, ATA_REG_LBA1);
    unsigned char ch = ata_inb(dev->io_base, ATA_REG_LBA2);

    if (cl == 0xFF && ch == 0xFF) {
        return 0;
    }

    if ((cl == 0x00 && ch == 0x00) ||
        (cl == 0x3C && ch == 0xC3)) {
        block_device_t* block = ata_device_create(dev);
        ata_drive_char++;
        ata_device_init(dev);
        block_device_register(block);

        return 1;
    }

    if ((cl == 0x14 && ch == 0xEB) ||
        (cl == 0x69 && ch == 0x96)) {
        dbg("CD-ROM detected.\n");
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
        ata_pci = device;
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
