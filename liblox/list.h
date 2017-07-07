#pragma once

#include <stddef.h>

#include "memory.h"

typedef struct list_node_t {
    struct list_node_t *prev;
    struct list_node_t *next;
    struct list_t *list;
    void *value;
} list_node_t;

typedef struct list_t {
    list_node_t *head;
    size_t size;
} list_t;

void list_init(list_t *list);
void list_init_node(list_node_t *node);

list_t* list_create(void);
list_node_t* list_create_node(void);

list_node_t* list_insert_after(list_node_t *node, void *value);
list_node_t* list_insert_node_after(list_node_t *node, list_node_t *entry);

list_node_t* list_insert_before(list_node_t *node, void *value);
list_node_t* list_insert_node_before(list_node_t *node, list_node_t *entry);

list_node_t* list_add(list_t *list, void *value);
list_node_t* list_add_node(list_t *list, list_node_t *entry);

list_node_t* list_get_at(list_t *list, size_t index);
void* list_get_value_at(list_t *list, size_t index);
void list_remove(list_node_t *node);
void list_free(list_t *list);

