/**
 * Kernel device registry.
 */
#pragma once

#include <liblox/common.h>
#include <liblox/list.h>
#include <liblox/tree.h>

#include "types.h"

/**
 * Device class.
 */
typedef uint32_t device_class_t;

/**
 * A device entry.
 */
typedef struct device_entry {
    /**
     * A device name.
     */
    char* name;

    /**
     * A device classification.
     */
    uint classifier;

    /**
     * Pointer to the device.
     */
    void* device;

    /**
     * Device tree node.
     */
    tree_node_t* node;
} device_entry_t;

/**
 * Get the device root.
 * @return device root.
 */
device_entry_t* device_root(void);

/**
 * Register a device with the device registry.
 * @param parent parent device.
 * @param name name of the device.
 * @param classifier device classifier.
 * @param device device pointer.
 * @return the device entry, if it was created.
 */
device_entry_t* device_register(
    device_entry_t* parent,
    char* name,
    device_class_t classifier,
    void* device
);

/**
 * Unregister a device.
 * @param device device entry.
 * @return if the device was unregistered.
 */
bool device_unregister(
    device_entry_t* device
);

/**
 * Find all devices with the given classifier.
 * @param classifier device classifier.
 * @return a list of device entries.
 */
list_t* device_query(
    device_class_t classifier
);

/**
 * Find a device with a given classifier by name.
 * @param name name of the device.
 * @param classifier device classifier.
 * @return the device, if found.
 */
device_entry_t* device_lookup(
    device_entry_t* root,
    char* name,
    device_class_t classifier
);

/**
 * Initialize the device registry.
 */
void device_registry_init(void);
