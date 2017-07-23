/**
 * Kernel mailbox event delivery.
 */
#pragma once

#include <kernel/spin.h>

/**
 * A mailbox delivery system.
 */
typedef struct mailbox {
    void* internal;
    spin_lock_t lock;
} mailbox_t;

typedef void (*mailbox_handler_t)(mailbox_t* box, void* event, void* extra);

void mailbox_add_handler(mailbox_t* box, mailbox_handler_t handler, void* extra);
void mailbox_remove_handler(mailbox_t* box, mailbox_handler_t handler);
void mailbox_deliver(mailbox_t* box, void* event);
void mailbox_deliver_async(mailbox_t* box, void* event);

mailbox_t* mailbox_create(void);
void mailbox_destroy(mailbox_t*);
