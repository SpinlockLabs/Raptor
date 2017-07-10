#include "events.h"

#include <liblox/common.h>
#include <liblox/hashmap.h>

#include <kernel/spin.h>

typedef struct event_dispatch_info {
    event_handler_t handler;
    void* extra;
} event_dispatch_info_t;

static hashmap_t* registry = NULL;
static spin_lock_t lock = {0};

void events_subsystem_init(void) {
    registry = hashmap_create(10);
    spin_init(lock);
}

void event_register_handler(char* type, event_handler_t handler, void* extra) {
    spin_lock(lock);
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

    spin_unlock(lock);
}

void event_unregister_handler(char* type, event_handler_t handler) {
    spin_lock(lock);
    if (!hashmap_has(registry, type)) {
        spin_unlock(lock);
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
        spin_unlock(lock);
        return;
    }

    spin_unlock(lock);
}

void event_dispatch(char* type, void* event) {
    spin_lock(lock);
    list_t* list = hashmap_get(registry, type);

    if (list == NULL) {
        spin_unlock(lock);
        return;
    }

    spin_lock(lock);
    list_for_each(node, list) {
        event_dispatch_info_t* info = node->value;

        info->handler(event, info->extra);
    }
    spin_unlock(lock);
}
