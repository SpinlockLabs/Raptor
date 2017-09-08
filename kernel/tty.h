#pragma once

#include <liblox/common.h>
#include <liblox/hashmap.h>

#include <stdint.h>

#include <kernel/dispatch/pipe.h>

typedef struct tty tty_t;

/**
 * Defines a function implemented by a TTY to write to the terminal.
 */
typedef void (*tty_write_func_t)(tty_t* tty, const uint8_t* buf, size_t size);

/**
 * Defines a structure that is passed down the
 * event pipe when a terminal has data available.
 */
typedef struct tty_read_event {
    /**
     * Data to be read.
     */
    uint8_t* data;

    /**
     * Size of the data.
     */
    size_t size;

    /**
     * TTY that has data available.
     */
    tty_t* tty;
} tty_read_event_t;

/**
 * Defines a function implemented by a TTY that is called
 * when the TTY is destroyed.
 */
typedef void (*tty_destroy_func_t)(tty_t* tty);

/**
 * Defines various flags that can be set by a TTY
 * to configure how the kernel will use it.
 */
typedef struct tty_flags {
    union {
        struct {
            /**
             * When this flag is enabled, the kernel will write kernel logs
             * to this TTY.
             */
            bool write_kernel_log : 1;

            /**
             * When this flag is enabled, the kernel will pass control on boot
             * to the kernel debug console.
             */
            bool allow_debug_console : 1;

            /**
             * When this flag is enabled, TTY owners should echo back
             * data when necessary to trigger certain terminal actions
             * or to show the data back onto the terminal.
             */
            bool echo : 1;

            /**
             * When this flag is enabled, the terminal is likely to have
             * non-textual data flowing through it, and no processing
             * should be done between the TTY and the owner.
             */
            bool raw : 1;
        };
        uint32_t value;
    };
} tty_flags_t;

/**
 * Defines various flags that can be changed to configure
 * how the kernel will call TTY operations.
 */
typedef struct tty_status {
    union {
        struct {
            /**
             * When this flag is enabled, the post write callback will be
             * called after the kernel dispatches a TTY write operation.
             */
            bool execute_post_write : 1;
        };
        uint32_t value;
    };
} tty_status_t;

typedef struct tty_private {
    /**
     * TTY (provider) specific data.
     */
    void* provider;

    /**
     * Owner (controller) specific data.
     */
    void* controller;
} tty_private_t;

/**
 * A terminal for communication.
 */
struct tty {
    /**
     * TTY name.
     */
    char* name;
    tty_flags_t flags;
    tty_status_t status;

    /**
     * TTY internal operations.
     */
    struct {
        tty_write_func_t write;
        tty_write_func_t post_write;
        tty_destroy_func_t destroy;
    } ops;

    /**
     * Event pipe for TTY reads.
     * Sends tty_read_event_t when data is available.
     */
    epipe_t reads;

    /**
     * Internal data for owners and controllers.
     */
    tty_private_t internal;
};

tty_t* tty_create(char* name);
void tty_init(tty_t* tty, char* name);
void tty_register(tty_t* tty);
tty_t* tty_get(char* name);
void tty_destroy(tty_t* tty);

void tty_write(tty_t* tty, uint8_t* buf, size_t size);
void tty_write_string(tty_t* tty, char* str);
void tty_printf(tty_t* tty, char* fmt, ...);

list_t* tty_get_names(void);
list_t* tty_get_all(void);

void tty_subsystem_init(void);
