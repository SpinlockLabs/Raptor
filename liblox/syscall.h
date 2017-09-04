#pragma once

#include <stdint.h>

typedef uint32_t syscall_id_t;
typedef int syscall_result_t;

/**
 * Exit the current process.
 */
#define SYSCALL_EXIT 1

/**
 * Write data to the current console.
 */
#define SYSCALL_CONSOLE_WRITE 2

syscall_result_t syscall(syscall_id_t id, ...);
