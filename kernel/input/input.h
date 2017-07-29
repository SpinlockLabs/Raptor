#pragma once

#include <stdint.h>

#include <liblox/common.h>
#include <liblox/list.h>

#include <kernel/dispatch/mailbox.h>

typedef struct input_device input_device_t;

/**
 * An ioctl operation function implemented by an input device.
 */
typedef int (*input_device_ioctl_op_t)(input_device_t*, ulong request, void*);

/**
 * A destroy function implemented by an input device.
 */
typedef bool (*input_device_destroy_op_t)(input_device_t*);

/**
 * Input device internal data.
 */
typedef struct input_device_private {
    /**
     * Input device provider data.
     */
    void* data;
} input_device_private_t;

/**
 * Input device operations.
 */
typedef struct input_device_ops {
    input_device_ioctl_op_t ioctl;
    input_device_destroy_op_t destroy;
} input_device_ops_t;

/**
 * Input event types.
 */
typedef enum input_event_type {
    /**
     * An unknown event.
     */
    INPUT_EVENT_TYPE_UNKNOWN,

    /**
     * A mouse location event.
     */
    INPUT_EVENT_TYPE_MOUSE_LOCATION,

    /**
     * A mouse click event.
     */
    INPUT_EVENT_TYPE_MOUSE_CLICK,

    /**
     * A key down event.
     */
    INPUT_EVENT_TYPE_KEY_DOWN,

    /**
     * A key up event.
     */
    INPUT_EVENT_TYPE_KEY_UP
} input_event_type_t;

/**
 * Input device event.
 */
typedef struct input_event {
    /**
     * Input event type.
     */
    input_event_type_t type;

    /**
     * Size of the event data.
     */
    size_t data_size;

    /**
     * Event data.
     */
    uint8_t data[];
} input_event_t;

/**
 * Input device classes.
 */
typedef enum input_device_class {
    /**
     * A mouse input device.
     */
    INPUT_DEV_CLASS_MOUSE,

    /**
     * A keyboard input device.
     */
    INPUT_DEV_CLASS_KEYBOARD
} input_device_class_t;

/**
 * Input device.
 */
struct input_device {
    /**
     * The name of the input device.
     */
    char* name;

    /**
     * Input device type.
     */
    input_device_class_t type;

    /**
     * Mailbox for the input events.
     *
     * This mailbox delivers events with
     * the type input_event_t.
     */
    mailbox_t* events;

    /**
     * Input device operations.
     */
    input_device_ops_t ops;

    /**
     * Internal data.
     */
    input_device_private_t internal;
};

/**
 * Find input devices with the given kind.
 * @param type input device class.
 * @return a list of devices.
 */
list_t* input_device_find(input_device_class_t type);

/**
 * Gets an input device by name.
 * @param name input device name.
 * @return input device, if found.
 */
input_device_t* input_device_get(char* name);

/**
 * Registers an input device.
 * @param device an input device.
 * @return whether the input device was registered.
 */
bool input_device_register(input_device_t* device);

/**
 * Destroys an input device.
 * @param device an input device.
 * @return whether the input device was destroyed.
 */
bool input_device_destroy(input_device_t* device);
