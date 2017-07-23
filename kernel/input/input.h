#pragma once

#include <stdint.h>

#include <liblox/common.h>
#include <kernel/dispatch/mailbox.h>

typedef struct input_device input_device_t;

typedef int (*input_device_ioctl_op_t)(input_device_t*, ulong request, void*);

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
} input_device_ops_t;

/**
 * Input event types.
 */
typedef enum input_event_type {
    INPUT_EVENT_TYPE_UNKNOWN,
    INPUT_EVENT_TYPE_MOUSE_LOCATION,
    INPUT_EVENT_TYPE_MOUSE_CLICK,
    INPUT_EVENT_TYPE_KEY_DOWN,
    INPUT_EVENT_TYPE_KEY_UP
} input_event_type_t;

/**
 * Input device event.
 */
typedef struct input_event {
    input_event_type_t type;
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
    char* name;

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
