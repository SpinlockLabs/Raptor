#pragma once

#include <liblox/common.h>

#include <stdint.h>

#include <kernel/arch.h>
#include <kernel/scheduler/scheduler.h>

/* A process identifier. */
typedef uint pid_t;

/* Process status states. */
typedef enum process_status {
    /* The process is currently running. */
    PROCESS_RUNNING,

    /* The process is waiting. */
    PROCESS_WAITING
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

    /* Scheduler state. */
    scheduler_state_t scheduler;
} process_t;

volatile pid_t process_get_next_pid(void);
volatile process_t* process_get_current(void);
