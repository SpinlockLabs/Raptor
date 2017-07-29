#include "block.h"

#include <liblox/hashmap.h>
#include <liblox/string.h>

#include <kernel/spin.h>
#include <kernel/panic.h>

#include <kernel/dispatch/events.h>
#include <kernel/device/registry.h>

#include "mbr.h"

static hashmap_t* registry = NULL;
static spin_lock_t lock = {0};

void block_device_subsystem_init(void) {
    registry = hashmap_create(5);

    block_device_mbr_subsystem_init();
}

block_device_t* block_device_create(char* name) {
    size_t name_size = strlen(name);

    if (name_size > 63) {
        printf(
            WARN "Failed to create block device "
                  "called '%s', name is bigger "
                  "than 63 characters.\n", name);
        return NULL;
    }

    block_device_t* device = zalloc(sizeof(block_device_t));
    memcpy(device->name, name, name_size);
    return device;
}

block_device_t* block_device_get(char* name) {
    spin_lock(lock);
    block_device_t* device = hashmap_get(registry, name);
    spin_unlock(lock);
    return device;
}

list_t* block_device_get_all(void) {
    return hashmap_values(registry);
}

block_device_error_t block_device_register(block_device_t* device) {
    spin_lock(lock);

    if (hashmap_has(registry, device->name)) {
        printf(
            WARN "Failed to register block device '%s': "
                 "device already exists!\n"
        );

        spin_unlock(lock);
        return BLOCK_DEVICE_ERROR_EXISTS;
    }

    hashmap_set(registry, device->name, device);
    spin_unlock(lock);

    device_register(device->name, DEVICE_CLASS_BLOCK, device);

    event_dispatch_async(
        "block-device:initialized",
        device
    );

    return BLOCK_DEVICE_ERROR_OK;
}

block_device_error_t block_device_destroy(block_device_t* device) {
    spin_lock(lock);

    char* name = device->name;
    block_device_error_t error = BLOCK_DEVICE_ERROR_OK;

    event_dispatch(
        "block-device:destroying",
        device
    );

    device_unregister(device_lookup(
        device->name,
        DEVICE_CLASS_BLOCK
    ));

    if (device->ops.destroy != NULL) {
        error = device->ops.destroy(device);
    }

    hashmap_remove(registry, device);
    event_dispatch_async(
        "block-device:destroyed",
        name
    );

    spin_unlock(lock);

    return error;
}

block_device_error_t block_device_read(
    block_device_t* device,
    size_t offset,
    uint8_t* buffer,
    size_t size
) {
    if (device->ops.read != NULL) {
        return device->ops.read(
            device,
            offset,
            buffer,
            size
        );
    }
    return BLOCK_DEVICE_ERROR_UNHANDLED;
}

block_device_error_t block_device_write(
    block_device_t* device,
    size_t offset,
    uint8_t* buffer,
    size_t size
) {
    if (device->ops.write != NULL) {
        return device->ops.write(
            device,
            offset,
            buffer,
            size
        );
    }
    return BLOCK_DEVICE_ERROR_UNHANDLED;
}

block_device_error_t block_device_ioctl(
    block_device_t* device,
    ulong request,
    void* ptr
) {
    if (device->ops.ioctl != NULL) {
        return device->ops.ioctl(
            device,
            request,
            ptr
        );
    }
    return BLOCK_DEVICE_ERROR_UNHANDLED;
}

block_device_error_t block_device_stat(
    block_device_t* device,
    block_device_stat_t* stat
) {
    if (device->ops.stat != NULL) {
        return device->ops.stat(
            device,
            stat
        );
    }
    return BLOCK_DEVICE_ERROR_UNHANDLED;
}
