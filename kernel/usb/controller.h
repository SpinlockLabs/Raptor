#pragma once

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
} usb_controller_t;
