#pragma once

typedef enum syscall_id {
    SYSCALL_NOP = 0,
    SYSCALL_EXIT = 1
} syscall_id_t;

long syscall(syscall_id_t id, ...);
