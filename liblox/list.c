#include "common.h"
#include "list.h"
#include "string.h"

void list_init_node(list_node_t *node) {
    memset(node, 0, sizeof(list_node_t));
}

void list_init(list_t *list) {
    memset(list, 0, sizeof(list_t));
}

list_node_t* list_create_node(void) {
    list_node_t *node = (list_node_t*) malloc(sizeof(list_node_t));

    ensure_allocated(node);

    list_init_node(node);
    return node;
}

list_t* list_create(void) {
    list_t *val = (list_t*) malloc(sizeof(list_t));

    ensure_allocated(val);

    list_init(val);
    return val;
}

list_node_t* list_insert_after(list_node_t *node, void *value) {
    list_node_t *entry = list_create_node();

    ensure_allocated(entry);

    entry->value = value;

    list_node_t *tmp = node->next;
    node->next = entry;
    entry->prev = node;
    entry->next = tmp;

    if (node->list != NULL) {
        node->list->size++;
    }

    return entry;
}

list_node_t* list_add(list_t *list, void *value) {
    list_node_t *entry = list_create_node();

    ensure_allocated(entry);

    entry->value = value;

    if (list->head == NULL) {
        list->head = entry;
        list->size = 1;
        return entry;
    }

    list_node_t *current = list->head;
    while (current->next != NULL) {
        current = current->next;
    }
    list_insert_after(current, entry);
    return entry;
}

list_node_t* list_insert_before(list_node_t *node, void *value) {
    list_node_t *entry = list_create_node();

    ensure_allocated(entry);

    entry->value = value;

    list_node_t *tmp = node->prev;
    node->prev = entry;
    entry->prev = tmp;
    entry->next = node;

    if (node->list != NULL) {
        node->list->size++;
    }

    return entry;
}

void list_remove(list_node_t *node) {
    node->prev = node->next;

    if (node->list != NULL) {
        node->list->size--;
    }

    free(node);
}
