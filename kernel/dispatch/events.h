/**
 * Kernel event dispatch subsystem.
 */
#pragma once

#include <stdint.h>

#include "event.h"

/**
 * Defines the type for event identifiers.
 */
typedef uint32_t event_type_t;

/**
 * Defines a handler for events.
 */
typedef void (*event_handler_t)(void* event, void* extra);

/**
 * Adds an event handler for the given event type.
 * @param type event type
 * @param handler event handler
 * @param extra extra data
 */
void event_add_handler(
    event_type_t type,
    event_handler_t handler,
    void* extra
);

/**
 * Removes an event handler for the given event type.
 * @param type event type
 * @param handler event handler
 * @param extra extra data
 */
void event_remove_handler(
    event_type_t type,
    event_handler_t handler,
    void* extra
);

/**
 * Dispatches an event to event handlers for the given type.
 * @param type event type
 * @param event event data
 */
void event_dispatch(event_type_t type, void* event);

/**
 * Dispatches an event to event handlers
 * for the given type asynchronously.
 * @param type event type
 * @param event event data
 */
void event_dispatch_async(event_type_t type, void* event);

/**
 * Initializes the global event system.
 */
void events_subsystem_init(void);
