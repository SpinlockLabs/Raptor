#pragma once

#include <kernel/device/registry.h>

typedef struct usb_controller usb_controller_t;

typedef struct usb_controller_private {
    /**
     * Controller data.
     */
    void* data;

    /**
     * Controller manager data.
     */
    void* manager;
} usb_controller_private_t;

/**
 * Operations implemented by a USB controller.
 */
typedef struct usb_controller_ops {
    /**
     * Implements polling of a USB controller.
     * @param controller controller
     */
    void (*poll)(usb_controller_t* controller);
} usb_controller_ops_t;

/**
 * USB controller.
 */
typedef struct usb_controller {
    /**
     * Unique name for the USB controller.
     */
    char name[64];

    /**
     * Internal-specific fields.
     */
    usb_controller_private_t internal;

    /**
     * Internal operations.
     */
    usb_controller_ops_t ops;

    /**
     * Device entry.
     */
    device_entry_t* entry;
} usb_controller_t;
