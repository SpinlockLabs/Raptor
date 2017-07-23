#pragma once

#include <liblox/common.h>

#include <stdint.h>
#include <stdbool.h>

/* Defines a kernel task function. */
typedef void (*ktask_func_t)(void*);

/* A task identifier. */
typedef ulong ktask_id;

/* Repeats a function on the kernel task queue at the given interval. */
ktask_id ktask_repeat(ulong ticks, ktask_func_t func, void* data);

/* Adds a function to the kernel task queue. */
ktask_id ktask_queue(ktask_func_t func, void* data);

/* Runs through the kernel task queue. */
void ktask_queue_flush(void);

/* Removes the kernel task with the given id from the queue. */
void ktask_cancel(ktask_id id);
