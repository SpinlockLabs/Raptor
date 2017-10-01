#include "events.h"

#include <liblox/hashmap.h>
#include <liblox/memory.h>

#include <kernel/spin.h>
#include <kernel/cpu/task.h>

typedef struct event_dispatch_info {
    event_handler_t handler;
    void* extra;
} event_dispatch_info_t;

static struct {
    hashmap_t registry;
    spin_lock_t lock;
} event_state;

#define LOCK() spin_lock(&event_state.lock)
#define UNLOCK() spin_unlock(&event_state.lock)

void events_subsystem_init(void) {
    hashmap_init_int(&event_state.registry, 10);
    spin_init(&event_state.lock);
}

void event_add_handler(event_type_t type, event_handler_t handler, void* extra) {
    LOCK();

    list_t* list;
    if (!hashmap_has(&event_state.registry, (void*) (uintptr_t) type)) {
        list = list_create();
        hashmap_set(&event_state.registry, (void*) (uintptr_t) type, list);
    } else {
        list = hashmap_get(&event_state.registry, (void*) (uintptr_t) type);
    }

    event_dispatch_info_t* info = zalloc(sizeof(event_dispatch_info_t));
    info->handler = handler;
    info->extra = extra;

    list_add(list, info);

    UNLOCK();
}

void event_remove_handler(event_type_t type, event_handler_t handler, void* extra) {
    LOCK();
    if (!hashmap_has(&event_state.registry, (void*) (uintptr_t) type)) {
        UNLOCK();
        return;
    }

    list_t* list = hashmap_get(&event_state.registry, (void*) (uintptr_t) type);

    list_for_each(node, list) {
        event_dispatch_info_t* info = node->value;

        if (info->handler != handler) {
            continue;
        }

        if (info->extra != extra) {
            continue;
        }

        list_remove(node);
        free(node);
        free(info);
        UNLOCK();
        return;
    }

    UNLOCK();
}

void event_dispatch(event_type_t type, void* event) {
     LOCK();

    if (!hashmap_has(&event_state.registry, (void*) (uintptr_t) type)) {
        UNLOCK();
        return;
    }

    list_t* list = hashmap_get(&event_state.registry, (void*) (uintptr_t) type);

    if (list == NULL) {
        UNLOCK();
        return;
    }

    UNLOCK();

    list_for_each(node, list) {
        event_dispatch_info_t* info = node->value;
        info->handler(event, info->extra);
    }
}

typedef struct event_dispatch_async_data {
    event_type_t type;
    void* event;
} event_dispatch_async_data_t;

static void event_dispatch_async_task(void* data) {
    event_dispatch_async_data_t* info = data;
    event_dispatch(info->type, info->event);
    free(info);
}

void event_dispatch_async(event_type_t type, void* event) {
    event_dispatch_async_data_t* data = zalloc(
        sizeof(event_dispatch_async_data_t)
    );

    data->type = type;
    data->event = event;
    ktask_queue(event_dispatch_async_task, data);
}
