#include "block.h"

#include <liblox/memory.h>
#include <liblox/hashmap.h>
#include <liblox/string.h>
#include <liblox/io.h>

#include <kernel/spin.h>

#include <kernel/dispatch/events.h>

#include "mbr.h"
#include "nulldev.h"

static struct {
    hashmap_t* registry;
    spin_lock_t lock;
} blockdev_subsystem;

#define REGISTRY blockdev_subsystem.registry
#define LOCK() spin_lock(&blockdev_subsystem.lock)
#define UNLOCK() spin_unlock(&blockdev_subsystem.lock)

void block_device_subsystem_init(void) {
    REGISTRY = hashmap_create(5);
    
    SET_SPIN_LOCK_LABEL(&blockdev_subsystem.lock, "Block Devices");

    block_device_mbr_subsystem_init();
    block_device_t* nulldev = null_block_dev_create("null");
    block_device_register(device_root(), nulldev);
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
    LOCK();
    block_device_t* device = hashmap_get(REGISTRY, name);
    UNLOCK();
    return device;
}

list_t* block_device_get_all(void) {
    LOCK();
    list_t* block_devices = hashmap_values(REGISTRY);
    UNLOCK();
    return block_devices;
}

block_device_error_t block_device_register(
    device_entry_t* parent,
    block_device_t* device
) {
    LOCK();

    if (hashmap_has(REGISTRY, device->name)) {
        printf(
            WARN "Failed to register block device '%s': "
                 "device already exists!\n"
        );

        UNLOCK();
        return BLOCK_DEVICE_ERROR_EXISTS;
    }

    hashmap_set(REGISTRY, device->name, device);
    UNLOCK();

    event_dispatch_async(
        EVENT_BLOCK_DEVICE_INITIALIZED,
        device
    );

    device_register(
        parent,
        device->name,
        DEVICE_CLASS_BLOCK,
        device
    );

    return BLOCK_DEVICE_ERROR_OK;
}

block_device_error_t block_device_destroy(block_device_t* device) {
    event_dispatch(
        EVENT_BLOCK_DEVICE_DESTROYING,
        device
    );

    if (device->entry != NULL) {
        device_unregister(device->entry);
    }

    LOCK();

    char* name = device->name;
    block_device_error_t error = BLOCK_DEVICE_ERROR_OK;

    if (device->ops.destroy != NULL) {
        error = device->ops.destroy(device);
    }

    hashmap_remove(REGISTRY, device);
    event_dispatch_async(
        EVENT_BLOCK_DEVICE_DESTROYED,
        name
    );

    UNLOCK();

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
