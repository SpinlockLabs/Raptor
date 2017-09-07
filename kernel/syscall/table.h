#pragma once

#include <liblox/hashmap.h>
#include <liblox/syscall.h>

/* Lox system call table. */
#define LOXCALL_TABLE 0

/**
 * Defines the maximum number of system calls in a given set.
 */
#define SYSCALL_TABLE_MAX_CALLS 512

/**
 * Defines the maximum number of system call sets.
 */
#define SYSCALL_TABLE_MAX_SETS 16

/**
 * A handler for a system call.
 */
typedef syscall_result_t (*syscall_handler_t)(
    uintptr_t* args
);

/**
 * System call set identifier.
 */
typedef uint32_t syscall_set_t;

/**
 * Add a system call to the given set.
 * @param set system call set
 * @param id system call id
 * @param handler system call handler
 * @return whether the call was added
 */
bool syscall_add(
    syscall_set_t set,
    syscall_id_t id,
    syscall_handler_t handler
);

/**
 * Remove a system call from the given set.
 * @param set system call set
 * @param id system call id
 * @return whether the call was removed
 */
bool syscall_remove(
    syscall_set_t set,
    syscall_id_t id
);

/**
 * Runs the given system call.
 * @param set system call set
 * @param id system call id
 * @param args null-terminated array of parameters
 * @return system call result
 */
syscall_result_t syscall_run(
    syscall_set_t set,
    syscall_id_t id,
    uintptr_t* args
);

/**
 * Initialize the system call tables.
 */
void syscall_init(void);
