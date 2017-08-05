#include "input.h"

#include <liblox/string.h>
#include <kernel/device/registry.h>

list_t* input_device_find(input_device_class_t type) {
    list_t* list = list_create();
    list_t* inputs = device_query(DEVICE_CLASS_INPUT);

    list_for_each(node, inputs) {
        device_entry_t* entry = node->value;
        input_device_t* device = entry->device;

        if (device->type == type) {
            list_add(list, device);
        }
    }

    list_free(inputs);

    return list;
}

input_device_t* input_device_get(char* name) {
    list_t* inputs = device_query(DEVICE_CLASS_INPUT);

    list_for_each(node, inputs) {
        device_entry_t* entry = node->value;

        if (strcmp(entry->name, name) == 0) {
            list_free(inputs);
            return entry->device;
        }
    }

    list_free(inputs);

    return NULL;
}

input_device_t* input_device_create(char* name, input_device_class_t type) {
    input_device_t* device = zalloc(sizeof(input_device_t));
    device->name = name;
    device->events = mailbox_create();
    device->type = type;
    return device;
}

bool input_device_register(input_device_t* device) {
    if (device == NULL) {
        return false;
    }

    if (device->events == NULL) {
        device->events = mailbox_create();
    }

    device_entry_t* entry = device_register(
        device->name,
        DEVICE_CLASS_INPUT,
        device
    );

    return entry != NULL;
}

bool input_device_destroy(input_device_t* device) {
    if (device == NULL) {
        return false;
    }

    bool result = device_unregister(device_lookup(
        device->name,
        DEVICE_CLASS_INPUT
    ));

    mailbox_destroy(device->events);
    device->events = NULL;

    if (result) {
        if (device->ops.destroy != NULL) {
            return device->ops.destroy(device);
        }
        free(device);
    }
    return result;
}
