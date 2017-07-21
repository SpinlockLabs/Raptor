#pragma once

#include <liblox/common.h>
#include <liblox/list.h>
#include <liblox/tree.h>

#include <stdint.h>

#include <kernel/arch.h>

/* A process identifier. */
typedef uint pid_t;

/* Process status states. */
typedef enum process_status {
    /* The process is currently running. */
    PROCESS_RUNNING,

    /* The process is waiting. */
    PROCESS_WAITING,

    /* The process is ready to run. */
    PROCESS_READY
} process_status_t;

/* Represents a process that can be scheduled. */
typedef struct process {
    /* The process identifier. */
    pid_t pid;

    /* The name of the process. */
    char* name;

    /* The process command line. */
    char** cmdline;

    /* Architecture-specific extensions. */
    arch_process_extensions_t arch;

    /* Current process status. */
    process_status_t status;

    /* CPU register state. */
    cpu_registers_t registers;
} process_t;

pid_t process_get_next_pid(void);
process_t* process_get_current(void);
list_t* process_get_all(void);
tree_t* process_get_tree(void);
