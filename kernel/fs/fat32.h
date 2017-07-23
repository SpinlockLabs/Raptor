#pragma once

#include <liblox/common.h>

#include <stdint.h>

typedef struct fat32_direntry {
    char name[8];
    char ext[3];

    uint8_t attrib;
    uint8_t user_attrib;

    char undelete;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t cluster_high;

    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t cluster_low;

    uint32_t file_size;
} packed fat32_direntry_t;

typedef struct fat_boot_sector {
    uint8_t boot_jmp[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t table_count;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t table_size_16;
    uint16_t sectors_per_track;
    uint16_t head_side_count;
    uint32_t hidden_sector_count;
    uint32_t total_sectors_32;

    uint8_t extended_section[54];
} packed fat_boot_sector_t;
