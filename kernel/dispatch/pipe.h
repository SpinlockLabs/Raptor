/**
 * Event pipe delivery system.
 */
#pragma once

#include <kernel/spin.h>

/**
 * An event pipe delivery system.
 */
typedef struct epipe {
    void* internal;
    spin_lock_t lock;
} epipe_t;

typedef void (*epipe_handler_t)(epipe_t* pipe, void* event, void* extra);

void epipe_add_handler(epipe_t* pipe, epipe_handler_t handler, void* extra);
void epipe_remove_handler(epipe_t* pipe, epipe_handler_t handler, void* extra);

void epipe_deliver(epipe_t* pipe, void* event);
void epipe_deliver_async(epipe_t* pipe, void* event);

epipe_t* epipe_create(void);

void epipe_init(epipe_t* pipe);
void epipe_destroy(epipe_t*);
