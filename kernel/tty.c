#include "tty.h"

#include <liblox/string.h>
#include <liblox/va_list.h>
#include <liblox/printf.h>
#include <liblox/memory.h>

static hashmap_t* tty_registry = NULL;

void tty_init(tty_t* tty, char* name) {
    memset(tty, 0, sizeof(tty_t));
    tty->name = name;
    tty->status.execute_post_write = true;
    epipe_init(&tty->reads);
}

tty_t* tty_create(char* name) {
    tty_t* tty = zalloc(sizeof(tty_t));
    tty_init(tty, name);
    return tty;
}

void tty_register(tty_t* tty) {
    if (tty_registry == NULL) {
        return;
    }

    hashmap_set(tty_registry, tty->name, tty);
}

void tty_write(tty_t* tty, uint8_t* buf, size_t size) {
    if (tty->ops.write != NULL) {
        tty->ops.write(tty, buf, size);
    }

    if (tty->status.execute_post_write && tty->ops.post_write != NULL) {
        tty->status.execute_post_write = false;
        tty->ops.post_write(tty, buf, size);
        tty->status.execute_post_write = true;
    }
}

void tty_write_string(tty_t* tty, char* str) {
    size_t size = strlen(str);
    tty_write(tty, (uint8_t*) str, size);
}

void tty_printf(tty_t* tty, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[2048];
    vasprintf(buf, fmt, args);
    va_end(args);
    tty_write(tty, (uint8_t*) buf, strlen(buf));
}

void tty_destroy(tty_t* tty) {
    if (tty->ops.destroy != NULL) {
        tty->ops.destroy(tty);
    }
}

tty_t* tty_get(char* name) {
    return hashmap_get(tty_registry, name);
}

list_t* tty_get_names(void) {
    return hashmap_keys(tty_registry);
}

list_t* tty_get_all(void) {
    return hashmap_values(tty_registry);
}

void tty_subsystem_init(void) {
    tty_registry = hashmap_create(10);
}

void tty_write_kernel_log_char(char c) {
    for (uint i = 0; i < tty_registry->size; ++i) {
        hashmap_entry_t* x = tty_registry->entries[i];
        while (x) {
            tty_t* tty = x->value;
            if (tty != NULL && tty->flags.write_kernel_log) {
                tty_write(tty, (uint8_t*) (&c), 1);
            }

            x = x->next;
        }
    }
}

void tty_write_kernel_log_string(char* msg) {
    size_t len = strlen(msg);
    for (uint i = 0; i < tty_registry->size; ++i) {
        hashmap_entry_t* x = tty_registry->entries[i];
        while (x) {
            tty_t* tty = x->value;
            if (tty != NULL && tty->flags.write_kernel_log) {
                tty_write(tty, (uint8_t*) msg, len);
            }

            x = x->next;
        }
    }
}
