#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct list_node {
    struct list* list;
    struct list_node* prev;
    struct list_node* next;
    void* value;
} list_node_t;

typedef struct list {
    list_node_t* head;
    list_node_t* tail;
    size_t size;
    union {
        struct {
            bool free_values : 1;
        };
        uint32_t flags;
    };
} list_t;

typedef int (*list_compare_t)(void* a, void* b);

void list_init(list_t* list);
void list_init_node(list_node_t* node);
list_t* list_create(void);
list_node_t* list_create_node(void);
list_node_t* list_insert_after(list_node_t* node, void* value);
list_node_t* list_insert_node_after(list_node_t* node, list_node_t* entry);
list_node_t* list_insert_before(list_node_t* node, void* value);
list_node_t* list_insert_node_before(list_node_t* node, list_node_t* entry);
list_node_t* list_add(list_t* list, void* value);
list_node_t* list_add_node(list_t* list, list_node_t* entry);
list_node_t* list_get_at(list_t* list, size_t index);
void* list_get_value_at(list_t* list, size_t index);
void list_remove(list_node_t* node);
list_node_t* list_dequeue(list_t* list);
void list_free(list_t* list);
void list_free_entries(list_t* list);
list_node_t* list_find(list_t* list, void* value);
list_t* list_diff(list_t* left, list_t* right);
bool list_contains(list_t* list, void* value);
void list_merge(list_t* target, list_t* source);
void list_swap(list_node_t* left, list_node_t* right);
void list_sort(list_t* list, list_compare_t compare);
list_t* list_pcreate(size_t count);

#define list_for_each(i, list) for (list_node_t* (i) = (list)->head; (i) != NULL; (i) = (i)->next)
