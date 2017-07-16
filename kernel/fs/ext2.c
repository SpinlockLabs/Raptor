#include <kernel/disk/block.h>
#include <kernel/spin.h>
#include <liblox/string.h>
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
    uint8_t bdg_offset;
    uint32_t inode_size;

    uint8_t* cache_data;
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

used static fs_node_t* ext2_mount(block_device_t* block) {
    ext2_fs_t* this = zalloc(sizeof(ext2_fs_t));

    this->device = block;
    this->block_size = 1024;

    this->superblock = zalloc(this->block_size);

    read_block(this, 1, (uint8_t*) this->superblock);
    return NULL;
}
