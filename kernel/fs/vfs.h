#pragma once

#include <stdint.h>

#include <liblox/common.h>
#include <kernel/disk/block.h>

#define VFS_MAX_NAME_SIZE 256
#define VFS_PATH_SEP '/'

typedef struct fs_node fs_node_t;
typedef enum fs_error fs_error_t;
typedef fs_node_t* (*vfs_filesystem_mounter_t)(block_device_t*);

enum fs_error {
    FS_ERROR_UNKNOWN = 0,
    FS_ERROR_OK,
    FS_ERROR_DOES_NOT_EXIST,
    FS_ERROR_EXISTS,
    FS_ERROR_BAD_DATA,
    FS_ERROR_TOO_BIG,
    FS_ERROR_IO_FAILED,
    FS_ERROR_BAD_STATE,
    FS_ERROR_BAD_CALL,
    FS_ERROR_NOT_IMPLEMENTED,
    FS_ERROR_BAD_TYPE
};

typedef enum fs_node_type {
    FS_TYPE_UNKNOWN,
    FS_TYPE_FILE,
    FS_TYPE_DIRECTORY,
    FS_TYPE_LINK
} fs_node_type_t;

typedef struct fs_stat {
    fs_node_type_t type;
    size_t size;
} fs_stat_t;

typedef struct fs_node_private {
    void* owner;
    void* vfs;
    void* tag;
} fs_node_private_t;

typedef struct fs_list_entry {
    fs_node_private_t internal;
    char name[VFS_MAX_NAME_SIZE];
} fs_list_entry_t;

#define FS_OP(name, ret, ...) \
    typedef ret (*fs_## name ##_op_t)(fs_node_t*, __VA_ARGS__);

FS_OP(read, fs_error_t, size_t, uint8_t*, size_t)
FS_OP(write, fs_error_t, size_t, uint8_t*, size_t)
FS_OP(stat, fs_error_t, fs_stat_t*)
FS_OP(ioctl, fs_error_t, long, void*)
FS_OP(list, fs_error_t, fs_list_entry_t**)
FS_OP(child, fs_error_t, char*, fs_node_t**)

#undef FS_OP

struct fs_node {
    char name[VFS_MAX_NAME_SIZE];

    fs_node_private_t internal;

    fs_stat_op_t stat;

    fs_read_op_t read;
    fs_write_op_t write;

    fs_list_op_t list;
    fs_child_op_t child;
};

typedef struct vfs_entry {
    char* name;
    fs_node_t* fs;
} vfs_entry_t;

void vfs_subsystem_init(void);

fs_node_t* fs_create_node(char* name);

fs_error_t vfs_mount_node(char*, fs_node_t*);
fs_error_t vfs_mount(block_device_t* block, char* type, char* path);
fs_error_t vfs_register_filesystem(char*, vfs_filesystem_mounter_t);

fs_node_t* fs_resolve(char* path);

fs_error_t fs_get_child(fs_node_t* parent, char* child, fs_node_t** node);
fs_error_t fs_read(fs_node_t* node, size_t offset, uint8_t* buffer, size_t size);
fs_error_t fs_stat(fs_node_t* node, fs_stat_t* stat);
fs_error_t fs_list(fs_node_t* node, fs_list_entry_t** entry);
