#include <kernel/disk/block.h>
#include <kernel/spin.h>
#include <liblox/string.h>
#include <liblox/io.h>
#include "ext2.h"
#include "vfs.h"

typedef struct ext2_fs {
    ext2_superblock_t* superblock;
    ext2_bgdescriptor_t* block_groups;
    block_device_t* device;
    size_t block_size;

    uint32_t pointers_per_block;
    uint32_t inodes_per_group;
    uint32_t block_group_count;

    ext2_disk_cache_entry_t* disk_cache;
    uint32_t cache_entries;
    uint32_t cache_time;

    spin_lock_t lock;

    uint8_t bgd_block_span;
    uint8_t bgd_offset;
    uint32_t inode_size;

    uint8_t* cache_data;
    fs_node_t* root;
} ext2_fs_t;

static uint get_cache_time(ext2_fs_t* this) {
    return this->cache_time++;
}

static fs_error_t cache_flush_dirty(ext2_fs_t* this, uint ent_no) {
    block_device_error_t error = block_device_write(
        this->device,
        this->disk_cache[ent_no].block_no * this->block_size,
        this->disk_cache[ent_no].block,
        this->block_size
    );

    if (error != BLOCK_DEVICE_ERROR_OK) {
        return FS_ERROR_IO_FAILED;
    }
    this->disk_cache[ent_no].dirty = 0;
    return FS_ERROR_OK;
}

static fs_error_t read_block(ext2_fs_t* this, uint block_id, uint8_t* buffer) {
    if (block_id == 0) {
        return FS_ERROR_BAD_CALL;
    }

    spin_lock(this->lock);
    if (this->disk_cache == NULL) {
        block_device_error_t error = block_device_read(
            this->device,
            block_id * this->block_size,
            buffer,
            this->block_size
        );

        spin_unlock(this->lock);

        if (error != BLOCK_DEVICE_ERROR_OK) {
            return FS_ERROR_IO_FAILED;
        }

        return FS_ERROR_OK;
    }

    int oldest = -1;
    uint oldest_age = UINT32_MAX;

    for (uint i = 0; i < this->cache_entries; i++) {
        if (this->disk_cache[i].block_no == block_id) {
            this->disk_cache[i].last_use = get_cache_time(this);
            memcpy(buffer, this->disk_cache[i].block, this->block_size);
            spin_unlock(this->lock);
            return FS_ERROR_OK;
        }

        if (this->disk_cache[i].last_use < oldest_age) {
            oldest = i;
            oldest_age = this->disk_cache[i].last_use;
        }
    }

    if (this->disk_cache[oldest].dirty) {
        cache_flush_dirty(this, (uint) oldest);
    }

    block_device_error_t error = block_device_read(
        this->device,
        block_id * this->block_size,
        this->disk_cache[oldest].block,
        this->block_size
    );

    if (error != BLOCK_DEVICE_ERROR_OK) {
        spin_unlock(this->lock);
        return FS_ERROR_IO_FAILED;
    }

    memcpy(buffer, this->disk_cache[oldest].block, this->block_size);

    this->disk_cache[oldest].block_no = block_id;
    this->disk_cache[oldest].last_use = get_cache_time(this);
    this->disk_cache[oldest].dirty = 0;

    spin_unlock(this->lock);

    return FS_ERROR_OK;
}

static void refresh_inode(
    ext2_fs_t* this,
    ext2_inodetable_t* table,
    uint32_t inode
) {
    uint32_t group = inode / this->inodes_per_group;
    if (group > this->block_group_count) {
        return;
    }

    uint32_t inode_table_block = this->block_groups[group].inode_table;
    inode -= group * this->inodes_per_group;
    uint32_t block_offset = ((inode - 1) * this->inode_size) / this->block_size;
    uint32_t offset_in_block = (inode - 1) - block_offset * (this->block_size / this->inode_size);
    uint8_t* buf = malloc(this->block_size);
    read_block(this, inode_table_block + block_offset, buf);
    ext2_inodetable_t* inodes = (ext2_inodetable_t*) buf;
    memcpy(
        table,
        (uint8_t*) ((uint32_t) inodes + offset_in_block * this->inode_size),
        this->inode_size
    );

    free(buf);
}

static ext2_inodetable_t* read_inode(
    ext2_fs_t* this,
    uint32_t inode
) {
    ext2_inodetable_t* table = zalloc(this->inode_size);
    refresh_inode(this, table, inode);
    return table;
}

used static fs_node_t* ext2_mount(block_device_t* block) {
    ext2_fs_t* this = zalloc(sizeof(ext2_fs_t));

    this->device = block;
    this->block_size = 1024;

    this->superblock = zalloc(this->block_size);

    read_block(this, 1, (uint8_t*) this->superblock);

    if (this->superblock->magic != EXT2_SUPER_MAGIC) {
        printf("EXT2 magic check failed for block device %s\n", block->name);
        return NULL;
    }
    this->inode_size = this->superblock->inode_size;
    if (this->inode_size == 0) {
        this->inode_size = 128;
    }

    this->block_size = (size_t) (1024 << this->superblock->log_block_size);
    this->cache_entries = 10240;

    if (this->block_size > 2048) {
        this->cache_entries /= 4;
    }

    this->pointers_per_block = this->block_size / 4;

    this->block_group_count = this->superblock->blocks_count /
                              this->superblock->blocks_per_group;

    if (this->superblock->blocks_per_group *
        this->block_group_count < this->superblock->blocks_count) {
        this->block_group_count += 1;
    }

    this->inodes_per_group = this->superblock->inodes_count / this->block_group_count;

    this->disk_cache = zalloc(sizeof(ext2_disk_cache_entry_t) * this->cache_entries);
    this->cache_data = zalloc(this->block_size * this->cache_entries);

    for (uint32_t i = 0; i < this->cache_entries; i++) {
        this->disk_cache[i].block = this->cache_data + i * this->block_size;
    }

    this->bgd_block_span = sizeof(ext2_bgdescriptor_t) *
                           this->block_group_count / this->block_size + 1;

    this->block_groups = zalloc(this->block_size * this->bgd_block_span);
    this->bgd_offset = 2;

    if (this->block_size > 1024) {
        this->bgd_offset = 1;
    }

    for (uint i = 0; i < this->bgd_block_span; i++) {
        read_block(
            this,
            this->bgd_offset + i,
            (uint8_t*) ((uint32_t) this->block_groups + this->block_size * i));
    }

    ext2_inodetable_t* root_inode = read_inode(this, 2);
    this->root = fs_create_node("");

    return this->root;
}
