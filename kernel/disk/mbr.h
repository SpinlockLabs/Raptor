#pragma once

#include <liblox/common.h>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <kernel/disk/block.h>

#define MBR_TABLE_ID 0x01

typedef struct mbr_partition {
    uint8_t status;
    uint8_t chs_first_sector[3];
    uint8_t type;
    uint8_t chs_last_sector[3];
    uint32_t lba_first_sector;
    uint32_t sector_count;
} packed mbr_partition_t;

typedef struct mbr {
    uint8_t bootstrap[446];
    mbr_partition_t partitions[4];
    uint8_t signature[2];
} packed mbr_t;

typedef struct mbr_block_table {
    block_device_t* children[4];
} mbr_block_table_t;

/* Checks a buffer to determine if it contains an MBR. */
bool mbr_check_signature(uint8_t*, size_t);

void block_device_mbr_subsystem_init(void);
