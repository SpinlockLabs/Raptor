#include "table.h"

static hashmap_t* syscall_table;

static void nop_value_free(void* val) {
    unused(val);
}

void syscall_init(void) {
    syscall_table = hashmap_create_int(2);
}

bool syscall_add(
    syscall_set_t set,
    syscall_id_t id,
    syscall_handler_t handler
) {
    hashmap_t* set_table = NULL;
    if (!hashmap_has(syscall_table, (void*) set)) {
        set_table = hashmap_create_int(16);
        set_table->value_free = nop_value_free;
        hashmap_set(syscall_table, (void*) set, set_table);
    } else {
        set_table = hashmap_get(syscall_table, (void*) set);
    }

    if (hashmap_has(set_table, (void*) id)) {
        return false;
    }

    hashmap_set(set_table, (void*) id, handler);
    return true;
}

bool syscall_remove(
    syscall_set_t set,
    syscall_id_t id
) {
    if (!hashmap_has(syscall_table, (void*) set)) {
        return false;
    }

    hashmap_t* set_table = hashmap_get(syscall_table, (void*) set);
    if (!hashmap_has(set_table, (void*) id)) {
        return false;
    }

    hashmap_remove(set_table, (void*) id);

    if (hashmap_count(set_table) == 0) {
        hashmap_remove(syscall_table, (void*) set);
    }
    return true;
}
