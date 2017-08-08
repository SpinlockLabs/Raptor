#pragma once

#include <liblox/common.h>
#include <liblox/list.h>
#include <liblox/tree.h>

#include <stdint.h>

#include <kernel/arch.h>

typedef struct process process_t;

extern void arch_process_init(process_t*);
extern void arch_process_init_kidle(process_t*);
extern void arch_process_run(process_t*);
extern void arch_process_switch_next(void);

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
struct process {
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
};

/**
 * Retrieves the next process PID.
 * @return process pid.
 */
pid_t process_get_next_pid(void);

/**
 * Gets the currently running process.
 * @return
 */
process_t* process_get_current(void);

/**
 * Gets a list of all the processes.
 * @return process list.
 */
list_t* process_get_all(void);

/**
 * Gets the process tree.
 * @return process tree.
 */
tree_t* process_get_tree(void);

/**
 * Get the next available process.
 * @return next available process.
 */
process_t* process_get_next(void);
