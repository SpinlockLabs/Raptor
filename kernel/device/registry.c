#include "registry.h"

#include <liblox/string.h>
#include <kernel/spin.h>
#include <kernel/dispatch/events.h>

static list_t* device_registry = NULL;
static spin_lock_t lock;

device_entry_t* device_register(
    char* name,
    uint classifier,
    void* device
) {
    if (name == NULL || device == NULL) {
        return NULL;
    }

    device_entry_t* existing = device_lookup(name, classifier);
    if (existing != NULL) {
        return NULL;
    }

    device_entry_t* entry = zalloc(sizeof(device_entry_t));
    entry->name = name;
    entry->classifier = classifier;
    entry->device = device;

    spin_lock(lock);
    list_add(device_registry, entry);
    spin_unlock(lock);

    event_dispatch_async("device:registered", entry);

    return entry;
}

bool device_unregister(
    device_entry_t* device
) {
    if (device == NULL) {
        return false;
    }

    list_node_t* node = list_find(device_registry, device);
    if (node != NULL) {
        event_dispatch("device:unregistered", node->value);

        spin_lock(lock);
        list_remove(node);
        free(node->value);
        free(node);
        spin_unlock(lock);
    }

    return node != NULL;
}

list_t* device_query(uint classifier) {
    list_t* list = list_create();

    spin_lock(lock);
    list_for_each(node, device_registry) {
        device_entry_t* dev = node->value;

        if (dev->classifier == classifier) {
            list_add(list, dev);
        }
    }

    spin_unlock(lock);

    return list;
}

device_entry_t* device_lookup(char* name, uint classifier) {
    spin_lock(lock);
    list_for_each(node, device_registry) {
        device_entry_t* dev = node->value;

        if (dev->classifier == classifier && strcmp(name, dev->name) == 0) {
            spin_unlock(lock);
            return dev;
        }
    }
    spin_unlock(lock);
    return NULL;
}

void device_registry_init(void) {
    device_registry = list_create();
    spin_init(lock);
}
