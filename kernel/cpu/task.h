#pragma once

#include <liblox/common.h>

#include <stdint.h>
#include <stdbool.h>

/* Defines a CPU task function. */
typedef void (*cpu_task_func_t)(void);

/* A task identifier. */
typedef ulong task_id;

/* Repeats a function on the CPU queue at the given interval. */
task_id cpu_task_repeat(ulong ticks, cpu_task_func_t func);

/* Adds a function to the CPU queue. */
task_id cpu_task_queue(cpu_task_func_t func);

/* Runs through the CPU task queue. */
void cpu_task_queue_flush(void);

/* Removes the CPU task with the given id from the queue. */
void cpu_task_cancel(task_id id);
