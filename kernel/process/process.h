#pragma once

#include <liblox/common.h>
#include <liblox/list.h>
#include <liblox/tree.h>

#include <stdint.h>

#include <kernel/arch.h>
#include <kernel/tty.h>

#define USER_STACK_BOTTOM 0xAFF00000
#define USER_STACK_TOP    0xB0000000

typedef struct process process_t;
typedef struct process_image process_image_t;

/**
 * Initializes the kernel idle process.
 */
extern void arch_process_init_kidle(process_t*);

/* A process identifier. */
typedef uint pid_t;

/* Process status states. */
typedef enum process_status {
    /* The process is currently running. */
    PROCESS_RUNNING,

    /* The process is waiting. */
    PROCESS_WAITING,

    /* The process is ready to run. */
    PROCESS_READY,

    /* The process is loading. */
    PROCESS_LOADING
} process_status_t;

/**
 * Process image.
 */
struct process_image {
    uintptr_t stack;
    uintptr_t size;
    uintptr_t entry;
    uintptr_t heap;
    uintptr_t heap_actual;
    uintptr_t user_stack;
    uintptr_t start;
};

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

    /* Process image. */
    process_image_t image;

    /**
     * Process tree node.
     */
    tree_node_t* node;

    /**
     * Process TTY.
     */
    tty_t* tty;
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
process_t* process_get_next_ready(void);

/**
 * Gets the kernel idle process.
 * @return kernel idle process.
 */
process_t* process_get_kidle(void);

/**
 * Sets the current process.
 */
void process_set_current(process_t*);

/**
 * Enqueue the process in the wait queue.
 */
void process_enqueue(process_t*);

/**
 * Initializes the process tree.
 */
void process_tree_init(void);
