#pragma once

#include <stdint.h>

/**
 * The type for a system call identifier.
 */
typedef uint32_t syscall_id_t;

/**
 * The type that a system call returns.
 */
typedef int syscall_result_t;

/**
 * Exit the current process.
 */
#define SYSCALL_EXIT 1

/**
 * Write data to the current console.
 */
#define SYSCALL_CONSOLE_WRITE 2

/**
 * Call a system call.
 * @param id system call id
 * @param ... parameters
 * @return result
 */
syscall_result_t syscall(syscall_id_t id, ...);
