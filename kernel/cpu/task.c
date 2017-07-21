#include "task.h"

#include <liblox/list.h>

#include <kernel/timer.h>

typedef struct ktask {
    ktask_id id;
    ulong repeat;
    ulong last_tick;
    ulong tick;
    ktask_func_t func;
    void* data;
} ktask_t;

static list_t* __task_queue = NULL;

static ktask_id ktask_counter = 0UL;

static void init_if_needed(void) {
    if (__task_queue == NULL) {
        __task_queue = list_create();
    }
}

ktask_id ktask_repeat(ulong ticks, ktask_func_t func, void* data) {
    init_if_needed();

    ktask_id id = ++ktask_counter;
    ktask_t *task = malloc(sizeof(ktask_t));

    task->id = id;
    task->repeat = ticks;
    task->tick = 0;
    task->last_tick = timer_get_ticks();
    task->func = func;
    task->data = data;

    list_add(__task_queue, task);
    return id;
}

ktask_id ktask_queue(ktask_func_t func, void* data) {
    init_if_needed();

    ktask_id id = ++ktask_counter;
    ktask_t *task = malloc(sizeof(ktask_t));

    task->id = id;
    task->repeat = 0;
    task->tick = 0;
    task->last_tick = timer_get_ticks();
    task->func = func;
    task->data = data;

    list_add(__task_queue, task);
    return id;
}

static volatile int __queue_state = 0;

static void __ktask_queue_flush(void) {
    if (__task_queue == NULL) {
        return;
    }

    ulong current_ticks = timer_get_ticks();
    list_node_t* node = __task_queue->head;

    while (node != NULL) {
        ktask_t* task = node->value;
        bool run = task->repeat == 0;

        if (task->repeat != 0) {
            task->tick += current_ticks - task->last_tick;
            run = task->tick >= task->repeat;
            task->last_tick = current_ticks;
        }

        if (run) {
            task->tick = 0;
            task->func(task->data);
        }

        if (task->repeat == 0) {
            list_node_t* next = node->next;
            list_remove(node);
            node = next;
        } else {
            node = node->next;
        }
    }
}

void ktask_queue_flush(void) {
    if (__queue_state > 0) {
        return;
    }

    __queue_state = 1;
    __ktask_queue_flush();
    __queue_state = 0;
}

void ktask_cancel(ktask_id id) {
    init_if_needed();

    list_node_t* node = __task_queue->head;

    while (node != NULL) {
        ktask_t* task = node->value;

        if (task->id == id) {
            list_remove(node);
            free(task);
            break;
        }

        node = node->next;
    }
}
