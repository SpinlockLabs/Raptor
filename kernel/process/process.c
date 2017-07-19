#include "process.h"

#include <liblox/tree.h>

#include <kernel/spin.h>

static tree_t* process_tree = NULL;
static list_t* process_list = NULL;
static pid_t next_pid = 0;

volatile static process_t* current_process = NULL;

static spin_lock_t tree_lock = {0};

void process_tree_init(void) {
    process_tree = tree_create();
    process_list = list_create();
}

volatile process_t* process_get_current(void) {
    return current_process;
}

volatile pid_t process_get_next_pid(void) {
    return next_pid++;
}
