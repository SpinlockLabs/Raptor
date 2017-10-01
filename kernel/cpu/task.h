#pragma once

#include <liblox/common.h>

#include <stdint.h>
#include <stdbool.h>

/**
 * Defines a kernel task function.
 */
typedef void (*ktask_func_t)(void* data);

/**
 * Defines the type for a kernel task id.
 */
typedef ulong ktask_id;

/**
 * Repeats a function on the kernel task queue at the given interval.
 * @param ticks number of ticks between the execution.
 * @param func function to call
 * @param data function data
 * @return ktask identifier
 */
ktask_id ktask_repeat(ulong ticks, ktask_func_t func, void* data);

/**
 * Adds a function to the kernel task queue.
 * @param func function to call
 * @param data function data
 * @return
 */
ktask_id ktask_queue(ktask_func_t func, void* data);

/**
 * Runs the kernel task queue.
 */
void ktask_queue_flush(void);

/**
 * Cancels the kernel task with the given id.
 * @param id ktask id
 */
void ktask_cancel(ktask_id id);
