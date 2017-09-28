#include "pipe.h"

#include <liblox/list.h>
#include <liblox/hashmap.h>
#include <liblox/memory.h>

#include <kernel/cpu/task.h>

typedef struct epipe_handler_entry {
    epipe_handler_t handler;
    void* extra;
} epipe_handler_entry_t;

void epipe_add_handler(epipe_t* pipe, epipe_handler_t handler, void* extra) {
    if (pipe == NULL) {
        return;
    }

    spin_lock(&pipe->lock);
    list_t* list = pipe->internal;

    epipe_handler_entry_t* info = zalloc(
        sizeof(epipe_handler_entry_t)
    );
    info->handler = handler;
    info->extra = extra;

    list_add(list, info);

    spin_unlock(&pipe->lock);
}

void epipe_remove_handler(epipe_t* pipe, epipe_handler_t handler) {
    if (pipe == NULL) {
        return;
    }

    spin_lock(&pipe->lock);

    list_t* list = pipe->internal;

    list_for_each(node, list) {
        epipe_handler_entry_t* info = node->value;
        if (info->handler != handler) {
            continue;
        }

        list_remove(node);
        free(node);
        free(info);
        spin_unlock(&pipe->lock);
        return;
    }

    spin_unlock(&pipe->lock);
}

void epipe_deliver(epipe_t* pipe, void* event) {
    if (pipe == NULL) {
        return;
    }

    spin_lock(&pipe->lock);
    list_t* list = pipe->internal;

    list_for_each(node, list) {
        epipe_handler_entry_t* info = node->value;
        info->handler(pipe, event, info->extra);
    }

    spin_unlock(&pipe->lock);
}

typedef struct epipe_deliver_async_data {
    epipe_t* pipe;
    void* event;
} epipe_deliver_async_data_t;

static void epipe_deliver_async_task(void* data) {
    epipe_deliver_async_data_t* info = data;
    epipe_deliver(info->pipe, info->event);
    free(info);
}

void epipe_deliver_async(epipe_t* pipe, void* event) {
    if (pipe == NULL) {
        return;
    }

    epipe_deliver_async_data_t* data = zalloc(
        sizeof(epipe_deliver_async_data_t)
    );

    data->pipe = pipe;
    data->event = event;
    ktask_queue(epipe_deliver_async_task, data);
}

epipe_t* epipe_create(void) {
    epipe_t* pipe = zalloc(sizeof(epipe_t));
    epipe_init(pipe);
    return pipe;
}

void epipe_init(epipe_t* pipe) {
    list_t* list = list_create();
    list->free_values = true;
    pipe->internal = list;

    spin_init(&pipe->lock);
}

void epipe_destroy(epipe_t* pipe) {
    if (pipe == NULL) {
        return;
    }

    spin_lock(&pipe->lock);
    list_t* list = pipe->internal;
    list_free(list);
    spin_unlock(&pipe->lock);

    free(pipe);
}
