/**
 * Kernel event dispatch subsystem.
 */
#pragma once

#include <stdint.h>
#include "event.h"

typedef uint32_t event_type;

typedef void (*event_handler_t)(void* event, void* extra);

void event_add_handler(event_type type, event_handler_t handler, void* extra);
void event_remove_handler(event_type type, event_handler_t handler);
void event_dispatch(event_type, void* event);
void event_dispatch_async(event_type, void* event);

void events_subsystem_init(void);
