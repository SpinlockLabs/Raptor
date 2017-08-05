#include "mbr.h"

#include <liblox/string.h>
#include <liblox/printf.h>

#include <kernel/dispatch/events.h>

bool mbr_check_signature(uint8_t* buffer, size_t size) {
    if (size < sizeof(mbr_t)) {
        return false;
    }
    mbr_t* mbr = (mbr_t*) buffer;
    return mbr->signature[0] == 0x55 && mbr->signature[1] == 0xAA;
}

typedef struct mbr_block_partition {
    mbr_partition_t mbr;
    block_device_t* parent;
} mbr_block_partition_t;

static block_device_error_t mbr_partition_block_read(
    block_device_t* block,
    size_t offset,
    uint8_t* buffer,
    size_t size
) {
    mbr_block_partition_t* part = block->internal.owner;
    block_device_t* disk = part->parent;
    uint32_t bsect = part->mbr.lba_first_sector;

    if (offset > (part->mbr.sector_count * 512)) {
        return BLOCK_DEVICE_ERROR_BAD_OFFSET;
    }

    size_t part_size = part->mbr.sector_count * 512;
    if (size > part_size) {
        return BLOCK_DEVICE_ERROR_OVERRUN;
    }

    size_t real_offset = (bsect * 512) + offset;
    return block_device_read(disk, real_offset, buffer, size);
}

static block_device_error_t mbr_partition_block_write(
    block_device_t* block,
    size_t offset,
    uint8_t* buffer,
    size_t size
) {
    mbr_block_partition_t* part = block->internal.owner;
    block_device_t* disk = part->parent;

    if (offset > (part->mbr.sector_count * 512)) {
        return BLOCK_DEVICE_ERROR_BAD_OFFSET;
    }

    size_t part_size = part->mbr.sector_count * 512;
    if (size > part_size) {
        return BLOCK_DEVICE_ERROR_OVERRUN;
    }

    size_t real_offset = (part_size) + offset;
    return block_device_write(disk, real_offset, buffer, size);
}

static block_device_error_t mbr_partition_block_stat(
    block_device_t* block,
    block_device_stat_t* stat
) {
    mbr_block_partition_t* part = block->internal.owner;
    stat->size = part->mbr.sector_count * 512;
    return BLOCK_DEVICE_ERROR_OK;
}

static block_device_t* mbr_create_block_partition(
    block_device_t* block,
    uint part_id,
    mbr_partition_t* part) {
    mbr_block_partition_t* internal = zalloc(sizeof(mbr_block_partition_t));
    internal->parent = block;
    memcpy(&internal->mbr, part, sizeof(mbr_partition_t));

    char pname[64] = {0};
    sprintf(pname, "%sp%d", block->name, part_id);
    block_device_t* pblock = block_device_create(pname);
    pblock->internal.owner = internal;
    pblock->ops.read = mbr_partition_block_read;
    pblock->ops.write = mbr_partition_block_write;
    pblock->ops.stat = mbr_partition_block_stat;
    block_device_register(pblock);
    return pblock;
}

static bool mbr_load_partitions(
    block_device_t* block,
    mbr_block_table_t* table,
    mbr_t* mbr
) {
    bool created = false;
    for (uint i = 0; i < 4; i++) {
        mbr_partition_t* part = &mbr->partitions[i];

        if ((part->status & 0x80) == 0) {
            continue;
        }

        block_device_t* dev = mbr_create_block_partition(block, i + 1, part);
        created = true;
        table->children[i] = dev;
    }

    return created;
}

static bool mbr_partition_probe(block_device_t* block) {
    uint8_t mbr[512] = {0};
    if (block_device_read(block, 0, mbr, 512) != BLOCK_DEVICE_ERROR_OK) {
        return false;
    }

    bool is_signature_valid = mbr_check_signature(mbr, 512);

    if (!is_signature_valid) {
        return false;
    }

    mbr_block_table_t* table = zalloc(sizeof(mbr_block_table_t));
    block->flags.partition_table_type = MBR_TABLE_ID;
    block->internal.table = table;

    return mbr_load_partitions(block, table, (mbr_t*) mbr);
}

static void mbr_partition_probe_handler(void* event, void* extra) {
    unused(extra);

    block_device_t* block = event;

    mbr_partition_probe(block);
}

static void mbr_destroy(block_device_t* device) {
    mbr_block_table_t* table = device->internal.table;
    for (uint i = 0; i < 4; i++) {
        if (table->children[i] == NULL) {
            continue;
        }

        block_device_t* dev = table->children[i];
        block_device_destroy(dev);
    }
}

static void mbr_destroy_parent_handler(void* event, void* extra) {
    unused(extra);
    block_device_t* block = event;

    if (block->flags.partition_table_type != MBR_TABLE_ID) {
        return;
    }

    mbr_destroy(block);
}

void block_device_mbr_subsystem_init(void) {
    event_add_handler(
        EVENT_BLOCK_DEVICE_INITIALIZED,
        mbr_partition_probe_handler,
        NULL
    );

    event_add_handler(
        EVENT_BLOCK_DEVICE_DESTROYING,
        mbr_destroy_parent_handler,
        NULL
    );

    event_add_handler(
        EVENT_BLOCK_DEVICE_PARTITION_PROBE,
        mbr_partition_probe_handler,
        NULL
    );
}
