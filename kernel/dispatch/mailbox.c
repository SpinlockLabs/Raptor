#include "mailbox.h"

#include <liblox/list.h>
#include <liblox/hashmap.h>

#include <kernel/cpu/task.h>

typedef struct mailbox_handler_entry {
    mailbox_handler_t handler;
    void* extra;
} mailbox_handler_entry_t;

void mailbox_add_handler(mailbox_t* box, mailbox_handler_t handler, void* extra) {
    spin_lock(box->lock);
    list_t* list = box->internal;

    mailbox_handler_entry_t* info = zalloc(sizeof(mailbox_handler_entry_t));
    info->handler = handler;
    info->extra = extra;

    list_add(list, info);

    spin_unlock(box->lock);
}

void mailbox_remove_handler(mailbox_t* box, mailbox_handler_t handler) {
    spin_lock(box->lock);

    list_t* list = box->internal;

    list_for_each(node, list) {
        mailbox_handler_entry_t* info = node->value;
        if (info->handler != handler) {
            continue;
        }

        list_remove(node);
        free(node);
        free(info);
        spin_unlock(box->lock);
        return;
    }

    spin_unlock(box->lock);
}

void mailbox_deliver(mailbox_t* box, void* event) {
    spin_lock(box->lock);
    list_t* list = box->internal;

    list_for_each(node, list) {
        mailbox_handler_entry_t* info = node->value;
        info->handler(box, event, info->extra);
    }

    spin_unlock(box->lock);
}

typedef struct mailbox_deliver_async_data {
    mailbox_t* box;
    void* event;
} mailbox_deliver_async_data_t;

static void mailbox_deliver_async_task(void* data) {
    mailbox_deliver_async_data_t* info = data;
    mailbox_deliver(info->box, info->event);
    free(info);
}

void mailbox_deliver_async(mailbox_t* box, void* event) {
    mailbox_deliver_async_data_t* data = zalloc(
        sizeof(mailbox_deliver_async_data_t)
    );

    data->box = box;
    data->event = event;
    ktask_queue(mailbox_deliver_async_task, data);
}

mailbox_t* mailbox_create(void) {
    mailbox_t* box = zalloc(sizeof(mailbox_t));
    list_t* list = list_create();
    list->free_values = true;
    box->internal = list;
    spin_init(box->lock);
    return box;
}

void mailbox_destroy(mailbox_t* box) {
    spin_lock(box->lock);
    list_t* list = box->internal;
    list_free(list);
    spin_unlock(box->lock);

    free(box);
}
