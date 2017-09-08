#include "table.h"

/**
 * The system call table is defined as the theoretical type
 * array(SYSCALL_TABLE_MAX_SETS)<array(SYSCALL_TABLE_MAX_CALLS)<syscall_handler_t>>
 *
 * These parameters will be tuned as development progresses.
 */
static syscall_handler_t syscall_table[SYSCALL_TABLE_MAX_SETS][SYSCALL_TABLE_MAX_CALLS];

void syscall_init(void) {
}

static bool do_check_set_and_id(syscall_set_t set, syscall_id_t id, bool add) {
    if (set >= SYSCALL_TABLE_MAX_SETS) {
        return false;
    }

    if (id > SYSCALL_TABLE_MAX_CALLS) {
        return false;
    }

    if (add) {
        if (syscall_table[set][id] != NULL) {
            return false;
        }
    } else {
        if (syscall_table[set][id] == NULL) {
            return false;
        }
    }

    return true;
}

bool syscall_add(
    syscall_set_t set,
    syscall_id_t id,
    syscall_handler_t handler
) {
    if (!do_check_set_and_id(set, id, true)) {
        return false;
    }

    syscall_table[set][id] = handler;
    return true;
}

bool syscall_remove(
    syscall_set_t set,
    syscall_id_t id
) {
    if (!do_check_set_and_id(set, id, false)) {
        return false;
    }

    syscall_table[set][id] = NULL;
    return true;
}

syscall_result_t syscall_run(
    syscall_set_t set,
    syscall_id_t id,
    uintptr_t* args
) {
    if (!do_check_set_and_id(set, id, false)) {
        return -1;
    }

    return syscall_table[set][id](args);
}
