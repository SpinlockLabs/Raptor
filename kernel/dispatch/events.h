/**
 * Kernel event dispatch subsystem.
 */
#pragma once

#include <stdint.h>
#include "event.h"

typedef uint32_t event_type_t;

typedef void (*event_handler_t)(void* event, void* extra);

void event_add_handler(event_type_t type, event_handler_t handler, void* extra);
void event_remove_handler(event_type_t type, event_handler_t handler);
void event_dispatch(event_type_t, void* event);
void event_dispatch_async(event_type_t, void* event);

void events_subsystem_init(void);
