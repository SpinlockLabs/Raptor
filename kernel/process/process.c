#include "process.h"

#include <liblox/tree.h>

#include <kernel/spin.h>

static tree_t* process_tree = NULL;
static list_t* process_list = NULL;
static list_t* wait_queue = NULL;

static process_t* kidle_process = NULL;
static volatile pid_t next_pid = 0;
static volatile process_t* current_process = NULL;

static spin_lock_t tree_lock = {0};

static process_t* create_kidle(void) {
    spin_lock(tree_lock);
    process_t* proc = zalloc(sizeof(process_t));
    proc->name = "[kernel]";
    proc->pid = 0;
    proc->cmdline = NULL;
    proc->status = PROCESS_RUNNING;
    return proc;
}

void process_tree_init(void) {
    spin_init(tree_lock);
    process_tree = tree_create();
    process_list = list_create();
    wait_queue = list_create();

    kidle_process = create_kidle();
}

process_t* process_get_current(void) {
    return (process_t*) current_process;
}

pid_t process_get_next_pid(void) {
    return (pid_t) next_pid++;
}

list_t* process_get_all(void) {
    return process_list;
}

tree_t* process_get_tree(void) {
    return process_tree;
}
