#pragma once

#include <stddef.h>

#include "memory.h"

typedef struct list_node {
    void *value;
    struct list_node *prev;
    struct list_node *next;
    struct list *list;
} list_node;

typedef struct list {
    list_node *head;
    size_t size;
} list;

void list_init_node(list_node *node);
list_node* list_create_node(void);
list_node* list_insert_after(list_node *node, void *value);
list_node* list_insert_before(list_node *node, void *value);
list_node* list_add(list *list, void *value);
void list_remove(list_node *node);
