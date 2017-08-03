#pragma once

#include <liblox/common.h>
#include <liblox/hashmap.h>

#include <stdint.h>

typedef struct tty tty_t;

typedef void (*tty_write_func_t)(tty_t* tty, const uint8_t* buf, size_t size);
typedef void (*tty_handle_read_func_t)(tty_t* tty, const uint8_t* buf, size_t size);
typedef void (*tty_destroy_func_t)(tty_t* tty);

typedef struct tty_flags {
    union {
        struct {
            bool write_kernel_log : 1;
            bool allow_debug_console : 1;
            bool echo : 1;
        };
        uint32_t value;
    };
} tty_flags_t;

typedef struct tty_status {
    union {
        struct {
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

struct tty {
    char* name;
    tty_flags_t flags;
    tty_status_t status;
    tty_write_func_t write;
    tty_write_func_t post_write;
    tty_handle_read_func_t handle_read;
    tty_destroy_func_t destroy;
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
