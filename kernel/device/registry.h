/**
 * Kernel device registry.
 */
#pragma once

#include <liblox/common.h>
#include <liblox/list.h>

#define DEVICE_CLASS_ALL 0x00
#define DEVICE_CLASS_UNKNOWN 0x01
#define DEVICE_CLASS_NETWORK 0x02
#define DEVICE_CLASS_INPUT 0x03
#define DEVICE_CLASS_BLOCK 0x04

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
} device_entry_t;

/**
 * Register a device with the device registry.
 * @param name name of the device.
 * @param classifier device classifier.
 * @param device device pointer.
 * @return
 */
device_entry_t* device_register(
    char* name,
    uint classifier,
    void* device
);

/**
 * Unregister a device.
 * @param device device entry.
 */
void device_unregister(
    device_entry_t* device
);

/**
 * Find all devices with the given classifier.
 * @param classifier device classifier.
 * @return a list of device entries.
 */
list_t* device_query(uint classifier);

/**
 * Find a device with a given classifier by name.
 * @param name name of the device.
 * @param classifier device classifier.
 * @return the device, if found.
 */
device_entry_t* device_lookup(char* name, uint classifier);

/**
 * Initialize the device registry.
 */
void device_registry_init(void);
