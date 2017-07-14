#pragma once

#include <stdint.h>
#include <stddef.h>

#include <liblox/common.h>
#include <liblox/list.h>

typedef struct block_device block_device_t;

typedef enum block_device_error {
    BLOCK_DEVICE_ERROR_UNKNOWN = 1,
    BLOCK_DEVICE_ERROR_OK,
    BLOCK_DEVICE_ERROR_BAD_OFFSET,
    BLOCK_DEVICE_ERROR_EXISTS,
    BLOCK_DEVICE_ERROR_OVERRUN,
    BLOCK_DEVICE_ERROR_UNHANDLED
} block_device_error_t;

typedef struct block_device_stat {
    size_t size;
} block_device_stat_t;

typedef block_device_error_t (*block_device_read_op_t)(
    block_device_t* device,
    size_t offset,
    uint8_t* buffer,
    size_t size
);

typedef block_device_error_t (*block_device_write_op_t)(
    block_device_t* device,
    size_t offset,
    uint8_t* buffer,
    size_t size
);

typedef block_device_error_t (*block_device_ioctl_op_t)(
    block_device_t* device,
    ulong request,
    void* ptr
);

typedef block_device_error_t (*block_device_stat_op_t)(
    block_device_t* device,
    block_device_stat_t*
);

typedef block_device_error_t (*block_device_destroy_op_t)(
    block_device_t* device
);

typedef struct block_device_ops {
    block_device_read_op_t read;
    block_device_write_op_t write;
    block_device_stat_op_t stat;
    block_device_ioctl_op_t ioctl;
    block_device_destroy_op_t destroy;
} block_device_ops_t;

typedef struct block_device_private {
    void* owner;
} block_device_private_t;

struct block_device {
    char name[64];
    block_device_ops_t ops;
    block_device_private_t internal;
};

block_device_t* block_device_create(char* name);

block_device_error_t block_device_register(block_device_t* device);
block_device_error_t block_device_destroy(block_device_t* device);

block_device_error_t block_device_stat(
    block_device_t* device,
    block_device_stat_t* stat
);

block_device_error_t block_device_read(
    block_device_t* device,
    size_t offset,
    uint8_t* buffer,
    size_t size
);

block_device_error_t block_device_write(
    block_device_t* device,
    size_t offset,
    uint8_t* buffer,
    size_t size
);

block_device_t* block_device_get(char* name);
list_t* block_device_get_all(void);

void block_device_subsystem_init(void);
