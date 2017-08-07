#include "ext2.h"

#include <liblox/string.h>
#include <liblox/io.h>
#include <liblox/memory.h>

#include <kernel/spin.h>
#include <kernel/cmdline.h>

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

    spin_lock(&this->lock);
    if (this->disk_cache == NULL) {
        block_device_error_t error = block_device_read(
            this->device,
            block_id * this->block_size,
            buffer,
            this->block_size
        );

        spin_unlock(&this->lock);

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
            spin_unlock(&this->lock);
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
        spin_unlock(&this->lock);
        return FS_ERROR_IO_FAILED;
    }

    memcpy(buffer, this->disk_cache[oldest].block, this->block_size);

    this->disk_cache[oldest].block_no = block_id;
    this->disk_cache[oldest].last_use = get_cache_time(this);
    this->disk_cache[oldest].dirty = 0;

    spin_unlock(&this->lock);

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

    if (this->block_size == 0) {
        return;
    }

    uint32_t inode_table_block = this->block_groups[group].inode_table;
    inode -= group * this->inodes_per_group;
    uint32_t block_offset = (uint32_t) (((inode - 1) * this->inode_size) / this->block_size);
    uint32_t offset_in_block = (uint32_t) ((inode - 1) - block_offset * (this->block_size / this->inode_size));
    uint8_t* buf = malloc(this->block_size);
    read_block(this, inode_table_block + block_offset, buf);
    ext2_inodetable_t* inodes = (ext2_inodetable_t*) buf;
    memcpy(
        table,
        (uint8_t*) ((uintptr_t) inodes + offset_in_block * this->inode_size),
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

static fs_error_t stat_dir_ext2(fs_node_t* node, fs_stat_t* stat) {
    unused(node);

    stat->type = FS_TYPE_DIRECTORY;
    stat->size = 0;

    return FS_ERROR_OK;
}

static fs_error_t stat_ext2(fs_node_t* node, fs_stat_t* stat) {
    unused(node);

    ext2_fs_t* this = node->internal.owner;
    uint32_t ind = (uint32_t) (uintptr_t) node->internal.tag;
    ext2_inodetable_t* inode = read_inode(this, ind);

    if (inode->mode & EXT2_S_IFDIR) {
        stat->type = FS_TYPE_DIRECTORY;
    } else {
        stat->type = FS_TYPE_FILE;
    }

    stat->size = inode->size;
    free(inode);

    return FS_ERROR_OK;
}

static uint get_block_number(ext2_fs_t* this, ext2_inodetable_t* inode, uint iblock) {
    uint p = this->pointers_per_block;

    /* We're going to do some crazy math in a bit... */
    uint a, b, c, d;

    uint8_t* tmp;

    if (iblock < EXT2_DIRECT_BLOCKS) {
        return inode->block[iblock];
    }

    if (iblock < EXT2_DIRECT_BLOCKS + p) {
        /* XXX what if inode->block[EXT2_DIRECT_BLOCKS] isn't set? */
        tmp = malloc(this->block_size);
        read_block(this, inode->block[EXT2_DIRECT_BLOCKS], tmp);
        uint out = ((uint32_t*) tmp)[iblock - EXT2_DIRECT_BLOCKS];
        free(tmp);
        return out;
    }

    if (iblock < EXT2_DIRECT_BLOCKS + p + p * p) {
        a = iblock - EXT2_DIRECT_BLOCKS;
        b = a - p;
        c = b / p;
        d = b - c * p;

        tmp = malloc(this->block_size);
        read_block(this, inode->block[EXT2_DIRECT_BLOCKS + 1], tmp);

        uint32_t nblock = ((uint32_t*) tmp)[c];
        read_block(this, nblock, tmp);

        uint out = ((uint32_t*) tmp)[d];
        free(tmp);
        return out;
    }

    if (iblock < EXT2_DIRECT_BLOCKS + p + p * p + p) {
        a = iblock - EXT2_DIRECT_BLOCKS;
        b = a - p;
        c = b - p * p;
        d = c / (p * p);

        uint e = c - d * p * p;
        uint f = e / p;
        uint g = e - f * p;

        tmp = malloc(this->block_size);
        read_block(this, inode->block[EXT2_DIRECT_BLOCKS + 2], tmp);

        uint32_t nblock = ((uint32_t*) tmp)[d];
        read_block(this, nblock, tmp);

        nblock = ((uint32_t*) tmp)[f];
        read_block(this, nblock, tmp);

        unsigned int out = ((uint32_t*) tmp)[g];
        free(tmp);
        return out;
    }

    printf(ERROR "EXT2 driver tried to read to a block number that was too high (%d)\n", iblock);

    return 0;
}

static uint inode_read_block(
    ext2_fs_t* this,
    ext2_inodetable_t* inode,
    uint block,
    uint8_t* buf) {
    size_t fn = 0;
    if (this->block_size != 0) {
        fn = inode->blocks / (this->block_size / 512);
    }

    if (block >= fn) {
        memset(buf, 0x00, this->block_size);
        printf(WARN "Tried to read an invalid block. Asked "
                    "for %d (0-indexed), but inode only has %d!\n",
               block,
               fn
        );
        return 0;
    }

    uint real_block = get_block_number(this, inode, block);
    read_block(this, real_block, buf);

    return real_block;
}

static ext2_dir_t* direntry_ext2(
    ext2_fs_t* this,
    ext2_inodetable_t* inode,
    uint32_t no,
    uint32_t index) {
    unused(no);

    uint8_t* block = malloc(this->block_size);
    uint8_t block_nr = 0;
    inode_read_block(this, inode, block_nr, block);
    uint32_t dir_offset = 0;
    uint32_t total_offset = 0;
    uint32_t dir_index = 0;

    while (total_offset < inode->size && dir_index <= index) {
        ext2_dir_t* d_ent = (ext2_dir_t*) ((uintptr_t) block + dir_offset);

        if (d_ent->inode != 0 && dir_index == index) {
            ext2_dir_t* out = zalloc(d_ent->rec_len);
            memcpy(out, d_ent, d_ent->rec_len);
            free(block);
            return out;
        }

        dir_offset += d_ent->rec_len;
        total_offset += d_ent->rec_len;

        if (d_ent->inode) {
            dir_index++;
        }

        if (dir_offset >= this->block_size) {
            block_nr++;
            dir_offset -= this->block_size;
            inode_read_block(this, inode, block_nr, block);
        }
    }

    free(block);
    return NULL;
}

static fs_error_t get_child_ext2(fs_node_t* node, char* name, fs_node_t** out);
static fs_error_t read_ext2(fs_node_t* node, size_t offset, uint8_t* buffer, size_t size);
static fs_error_t list_ext2(fs_node_t* node, fs_list_entry_t** eout);

static fs_error_t node_from_file(
    ext2_fs_t* this,
    ext2_inodetable_t* inode,
    ext2_dir_t* direntry,
    fs_node_t* fnode) {
    unused(inode);

    if (!fnode) {
        return FS_ERROR_BAD_STATE;
    }

    fnode->internal.owner = this;
    fnode->internal.tag = (void*) (uintptr_t) direntry->inode;

    memcpy(&fnode->name, &direntry->name, direntry->name_len);
    fnode->name[direntry->name_len] = '\0';

    fnode->stat = stat_ext2;
    fnode->child = get_child_ext2;
    fnode->read = read_ext2;
    fnode->list = list_ext2;

    return FS_ERROR_OK;
}

static fs_error_t get_child_ext2(fs_node_t* node, char* name, fs_node_t** out) {
    ext2_fs_t* this = node->internal.owner;

    ext2_inodetable_t* inode = read_inode(this, (uint32_t) (uintptr_t) node->internal.tag);
    uint8_t* block = malloc(this->block_size);
    ext2_dir_t* direntry = NULL;
    uint8_t block_nr = 0;
    inode_read_block(this, inode, block_nr, block);
    uint32_t dir_offset = 0;
    uint32_t total_offset = 0;

    while (total_offset < inode->size) {
        if (dir_offset >= this->block_size) {
            block_nr++;
            dir_offset -= this->block_size;
            inode_read_block(this, inode, block_nr, block);
        }
        ext2_dir_t* d_ent = (ext2_dir_t*) ((uintptr_t) block + dir_offset);

        if (d_ent->inode == 0 || strlen(name) != d_ent->name_len) {
            dir_offset += d_ent->rec_len;
            total_offset += d_ent->rec_len;

            continue;
        }

        char* dname = malloc(sizeof(char) * (d_ent->name_len + 1));
        memcpy(dname, &(d_ent->name), d_ent->name_len);
        dname[d_ent->name_len] = '\0';
        if (!strcmp(dname, name)) {
            free(dname);
            direntry = malloc(d_ent->rec_len);
            memcpy(direntry, d_ent, d_ent->rec_len);
            break;
        }
        free(dname);

        dir_offset += d_ent->rec_len;
        total_offset += d_ent->rec_len;
    }
    free(inode);

    if (!direntry) {
        free(block);
        return FS_ERROR_DOES_NOT_EXIST;
    }

    fs_node_t* outnode = zalloc(sizeof(fs_node_t));

    inode = read_inode(this, direntry->inode);

    if (!node_from_file(this, inode, direntry, outnode)) {
        free(direntry);
        free(outnode);
        free(block);
        free(inode);
        return FS_ERROR_BAD_STATE;
    }

    free(direntry);
    free(inode);
    free(block);
    *out = outnode;
    return FS_ERROR_OK;
}

static fs_error_t list_ext2(fs_node_t* node, fs_list_entry_t** eout) {
    ext2_fs_t* this = (ext2_fs_t*) node->internal.owner;
    ext2_inodetable_t* inode = read_inode(this, (uint32_t) (uintptr_t) node->internal.tag);
    if ((inode->mode & EXT2_S_IFDIR) == 0) {
        free(inode);
        return FS_ERROR_BAD_TYPE;
    }

    uint32_t index = 0;
    if (*eout != NULL) {
        index = (uint32_t) (uintptr_t) (*eout)->internal.tag;
        free(*eout);
    }

    ext2_dir_t* dir = direntry_ext2(
        this,
        inode,
        (uint32_t) (uintptr_t) node->internal.tag,
        index
    );

    if (dir == NULL) {
        *eout = NULL;
        free(inode);
        return FS_ERROR_OK;
    }

    fs_list_entry_t* entry = zalloc(sizeof(fs_list_entry_t));
    memcpy(&entry->name, dir->name, dir->name_len);
    entry->internal.owner = this;
    entry->internal.tag = (void*) (uintptr_t) dir->inode;
    *eout = entry;
    free(dir);
    free(inode);
    return FS_ERROR_OK;
}

static fs_error_t read_ext2(fs_node_t* node, size_t offset, uint8_t* buffer, size_t size) {
    ext2_fs_t* this = node->internal.owner;
    ext2_inodetable_t* inode = read_inode(this, (uint32_t) (uintptr_t) node->internal.tag);
    uint32_t end;
    if (inode->size == 0) {
        free(inode);
        return FS_ERROR_OK;
    }

    if (offset + size > inode->size) {
        end = inode->size;
    } else {
        end = (uint32_t) (offset + size);
    }
    uint32_t start_block = (uint32_t) (offset / this->block_size);
    uint32_t end_block = (uint32_t) (end / this->block_size);
    uint32_t end_size = (uint32_t) (end - end_block * this->block_size);
    uint32_t size_to_read = (uint32_t) (end - offset);

    uint8_t* buf = malloc(this->block_size);
    if (start_block == end_block) {
        inode_read_block(this, inode, start_block, buf);
        memcpy(buffer, (uint8_t*) (((uintptr_t) buf) + (offset % this->block_size)), size_to_read);
    } else {
        uint32_t blocks_read = 0;
        for (uint32_t block_offset = start_block; block_offset < end_block; block_offset++, blocks_read++) {
            if (block_offset == start_block) {
                inode_read_block(this, inode, block_offset, buf);
                memcpy(buffer, (uint8_t*) (((uintptr_t) buf) + (offset % this->block_size)),
                       this->block_size - (offset % this->block_size));
            } else {
                inode_read_block(this, inode, block_offset, buf);
                memcpy(buffer + this->block_size * blocks_read - (offset % this->block_size), buf, this->block_size);
            }
        }

        if (end_size) {
            inode_read_block(this, inode, end_block, buf);
            memcpy(buffer + this->block_size * blocks_read - (offset % this->block_size), buf, end_size);
        }
    }
    free(inode);
    free(buf);
    return FS_ERROR_OK;
}

static uint32_t ext2_root(ext2_fs_t* this, ext2_inodetable_t* inode, fs_node_t* node) {
    unused(inode);

    if (node == NULL) {
        return 0;
    }

    node->internal.owner = this;
    node->internal.tag = (void*) 2;
    node->name[0] = '/';
    node->name[1] = '\0';
    node->stat = stat_dir_ext2;
    node->child = get_child_ext2;
    node->list = list_ext2;

    return 1;
}

static fs_node_t* mount_ext2(block_device_t* block) {
    ext2_fs_t* this = zalloc(sizeof(ext2_fs_t));

    this->device = block;
    this->block_size = 1024;

    this->superblock = zalloc(this->block_size);

    fs_error_t error = read_block(this, 1, (uint8_t*) this->superblock);

    if (error != FS_ERROR_OK) {
        printf(WARN "Failed to read EXT2 superblock from block device %s\n", block->name);
        return NULL;
    }

    if (this->superblock->magic != EXT2_SUPER_MAGIC) {
        printf(WARN "EXT2 magic check failed for block device %s\n", block->name);
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

    this->pointers_per_block = (uint32_t) (this->block_size / 4);

    this->block_group_count = this->superblock->blocks_count /
                              this->superblock->blocks_per_group;

    if (this->superblock->blocks_per_group *
        this->block_group_count < this->superblock->blocks_count) {
        this->block_group_count += 1;
    }

    this->inodes_per_group = this->superblock->inodes_count / this->block_group_count;

    if (cmdline_bool_flag("ext2.enable-cache")) {
        this->disk_cache = zalloc(sizeof(ext2_disk_cache_entry_t) * this->cache_entries);
        this->cache_data = zalloc(this->block_size * this->cache_entries);

        for (uint32_t i = 0; i < this->cache_entries; i++) {
            this->disk_cache[i].block = this->cache_data + i * this->block_size;
        }
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
            (uint8_t*) ((uintptr_t) this->block_groups + this->block_size * i));
    }

    ext2_inodetable_t* root_inode = read_inode(this, 2);
    this->root = fs_create_node("/");

    if (!ext2_root(this, root_inode, this->root)) {
        free(root_inode);
        return NULL;
    }

    free(root_inode);
    return this->root;
}

static fs_node_t* ext2_mount(block_device_t* block) {
    return mount_ext2(block);
}

void ext2_filesystem_init(void) {
    vfs_register_filesystem("ext2", ext2_mount);
}
