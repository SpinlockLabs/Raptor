#include "task.h"

#include <kernel/timer.h>

#include <liblox/list.h>
#include <kernel/arch/x86/irq.h>

typedef struct cpu_task_t {
    task_id id;
    ulong repeat;
    ulong last_tick;
    ulong tick;
    cpu_task_func_t func;
    void* data;
} cpu_task_t;

static list_t *__cpu_task_queue = NULL;

static task_id cpu_task_counter = 0UL;

static void cpu_task_queue_init_if_needed(void) {
    if (__cpu_task_queue == NULL) {
        __cpu_task_queue = list_create();
    }
}

task_id cpu_task_repeat(ulong ticks, cpu_task_func_t func, void* data) {
    cpu_task_queue_init_if_needed();

    task_id id = ++cpu_task_counter;
    cpu_task_t *task = malloc(sizeof(cpu_task_t));

    task->id = id;
    task->repeat = ticks;
    task->tick = 0;
    task->last_tick = timer_get_ticks();
    task->func = func;
    task->data = data;

    list_add(__cpu_task_queue, task);
    return id;
}

task_id cpu_task_queue(cpu_task_func_t func, void* data) {
    cpu_task_queue_init_if_needed();

    task_id id = ++cpu_task_counter;
    cpu_task_t *task = malloc(sizeof(cpu_task_t));

    task->id = id;
    task->repeat = 0;
    task->tick = 0;
    task->last_tick = timer_get_ticks();
    task->func = func;
    task->data = data;

    list_add(__cpu_task_queue, task);
    return id;
}

static volatile int __cpu_queue_state = 0;

static void __cpu_task_queue_flush(void) {
    if (__cpu_task_queue == NULL) {
        return;
    }

    ulong current_ticks = timer_get_ticks();
    list_node_t* node = __cpu_task_queue->head;

    while (node != NULL) {
        cpu_task_t *task = (cpu_task_t*) node->value;
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

void cpu_task_queue_flush(void) {
    if (__cpu_queue_state > 0) {
        return;
    }

    __cpu_queue_state = 1;
    __cpu_task_queue_flush();
    __cpu_queue_state = 0;
}

void cpu_task_cancel(task_id id) {
    cpu_task_queue_init_if_needed();

    list_node_t* node = __cpu_task_queue->head;

    while (node != NULL) {
        cpu_task_t *task = (cpu_task_t*) node->value;

        if (task->id == id) {
            list_remove(node);
            free(task);
            break;
        }

        node = node->next;
    }
}
