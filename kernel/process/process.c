#include "process.h"

#include <liblox/memory.h>

static tree_t* process_tree = NULL;
static list_t* process_list = NULL;
static list_t* wait_queue = NULL;

static process_t* kidle_process = NULL;
static volatile pid_t next_pid = 0;
static volatile process_t* current_process = NULL;

static spin_lock_t tree_lock;

static process_t* create_kidle(void) {
    spin_lock(&tree_lock);
    process_t* proc = zalloc(sizeof(process_t));
    proc->name = "[idle]";
    proc->pid = process_get_next_pid();
    proc->cmdline = NULL;
    proc->status = PROCESS_RUNNING;

    arch_process_init_kidle(proc);
    list_add(process_list, proc);
    spin_unlock(&tree_lock);

    return proc;
}

void process_tree_init(void) {
    spin_init(&tree_lock);
    process_tree = tree_create();
    process_list = list_create();
    wait_queue = list_create();

    kidle_process = create_kidle();
}

process_t* process_get_current(void) {
    return (process_t*) current_process;
}

pid_t process_get_next_pid(void) {
    return next_pid++;
}

list_t* process_get_all(void) {
    return process_list;
}

tree_t* process_get_tree(void) {
    return process_tree;
}

process_t* process_get_next_ready(void) {
    process_t* next = NULL;
    do {
        if (wait_queue->size == 0) {
            return kidle_process;
        }

        list_node_t* node = list_dequeue(wait_queue);
        next = node->value;
        free(node);
    } while (next->status == PROCESS_STOPPED);
    return next;
}

void process_enqueue(process_t* process) {
    process->status = PROCESS_READY;
    list_add(wait_queue, process);
}

void process_set_current(process_t* process) {
    current_process = process;
}

process_t* process_get_kidle(void) {
    return kidle_process;
}
