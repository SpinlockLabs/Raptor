#pragma once

#include <stdint.h>
#include <stddef.h>

#include <liblox/common.h>

typedef enum block_device_error {
    BLOCK_DEVICE_ERROR_UNKNOWN = 1,
    BLOCK_DEVICE_ERROR_BAD_OFFSET,
    BLOCK_DEVICE_ERROR_OVERRUN,
    BLOCK_DEVICE_ERROR_OK
} block_device_error_t;

typedef block_device_error_t (*block_device_read_op_t)(
    struct block_device* device,
    size_t offset,
    uint8_t* buffer,
    size_t size
);

typedef block_device_error_t (*block_device_write_op_t)(
    struct block_device* device,
    size_t offset,
    uint8_t* buffer,
    size_t size
);

typedef block_device_error_t (*block_device_ioctl_t)(
    struct block_device* device,
    ulong request,
    void* ptr
);

typedef struct block_device_ops {
    block_device_read_op_t read;
    block_device_write_op_t write;
    block_device_ioctl_t ioctl;
} block_device_ops_t;

typedef struct block_device_private {
    void* owner;
} block_device_private_t;

typedef struct block_device {
    char name[64];
    block_device_ops_t ops;
    block_device_private_t private;
} block_device_t;
