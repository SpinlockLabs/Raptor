/**
 * Kernel event dispatch subsystem.
 */
#pragma once

typedef void (*event_handler_t)(void* event, void* extra);

void event_register_handler(char* type, event_handler_t handler, void* extra);
void event_unregister_handler(char* type, event_handler_t handler);
void event_dispatch(char* type, void* event);

void events_subsystem_init(void);
