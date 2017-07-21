#include "process.h"

#include <liblox/tree.h>

#include <kernel/spin.h>

static tree_t* process_tree = NULL;
static list_t* process_list = NULL;

static volatile pid_t next_pid = 0;
static volatile process_t* current_process = NULL;

static spin_lock_t tree_lock = {0};

void process_tree_init(void) {
    process_tree = tree_create();
    process_list = list_create();
}

process_t* process_get_current(void) {
    return (process_t*) current_process;
}

pid_t process_get_next_pid(void) {
    return (pid_t) next_pid++;
}
