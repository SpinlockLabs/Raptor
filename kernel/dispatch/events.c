#include "events.h"

#include <liblox/common.h>
#include <liblox/hashmap.h>

#include <kernel/panic.h>
#include <kernel/spin.h>
#include <kernel/cpu/task.h>

typedef struct event_dispatch_info {
    event_handler_t handler;
    void* extra;
} event_dispatch_info_t;

static hashmap_t* registry = NULL;
static spin_lock_t registry_lock;

void events_subsystem_init(void) {
    registry = hashmap_create(10);

    if (registry == NULL) {
        panic("Failed to create the events subsystem registry.");
    }

    spin_init(&registry_lock);
}

void event_add_handler(char* type, event_handler_t handler, void* extra) {
    spin_lock(&registry_lock);
    list_t* list = NULL;
    if (!hashmap_has(registry, type)) {
        list = list_create();
        hashmap_set(registry, type, list);
    } else {
        list = hashmap_get(registry, type);
    }

    event_dispatch_info_t* info = zalloc(sizeof(event_dispatch_info_t));
    info->handler = handler;
    info->extra = extra;

    list_add(list, info);

    spin_unlock(&registry_lock);
}

void event_remove_handler(char* type, event_handler_t handler) {
    spin_lock(&registry_lock);
    if (!hashmap_has(registry, type)) {
        spin_unlock(&registry_lock);
        return;
    }

    list_t* list = hashmap_get(registry, type);

    list_for_each(node, list) {
        event_dispatch_info_t* info = node->value;
        if (info->handler != handler) {
            continue;
        }

        list_remove(node);
        free(node);
        free(info);
        spin_unlock(&registry_lock);
        return;
    }

    spin_unlock(&registry_lock);
}

void event_dispatch(char* type, void* event) {
    spin_lock(&registry_lock);

    if (!hashmap_has(registry, type)) {
        spin_unlock(&registry_lock);
        return;
    }

    list_t* list = hashmap_get(registry, type);

    if (list == NULL) {
        spin_unlock(&registry_lock);
        return;
    }

    spin_unlock(&registry_lock);

    list_for_each(node, list) {
        event_dispatch_info_t* info = node->value;
        info->handler(event, info->extra);
    }
}

typedef struct event_dispatch_async_data {
    char* type;
    void* event;
} event_dispatch_async_data_t;

static void event_dispatch_async_task(void* data) {
    event_dispatch_async_data_t* info = data;
    event_dispatch(info->type, info->event);
    free(info);
}

void event_dispatch_async(char* type, void* event) {
    event_dispatch_async_data_t* data = zalloc(
        sizeof(event_dispatch_async_data_t)
    );

    data->type = type;
    data->event = event;
    ktask_queue(event_dispatch_async_task, data);
}
