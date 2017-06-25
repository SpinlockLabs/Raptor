#include "kcommon.h"
#include "list.h"
#include "string.h"

void list_init_node(list_node *node) {
    memset(node, 0, sizeof(list_node));
}

void list_init(list *list) {
    memset(list, 0, sizeof(list));
}

list_node* list_create_node(void) {
    list_node *node = malloc(sizeof(list_node));

    ensure_allocated(node);

    list_init_node(node);
    return node;
}

list* list_create(void) {
    list *list = malloc(sizeof(list));

    ensure_allocated(list);

    list_init(list);
    return list;
}

list_node* list_insert_after(list_node *node, void *value) {
    list_node *entry = list_create_node();

    ensure_allocated(entry);

    entry->value = value;

    list_node *tmp = node->next;
    node->next = entry;
    entry->prev = node;
    entry->next = tmp;

    if (node->list != NULL) {
        node->list->size++;
    }

    return entry;
}

list_node* list_add(list *list, void *value) {
    list_node *entry = list_create_node();

    ensure_allocated(entry);

    entry->value = value;

    if (list->head == NULL) {
        list->head = entry;
        list->size = 1;
        return entry;
    }

    list_node *current = list->head;
    while (current->next != NULL) {
        current = current->next;
    }
    list_insert_after(current, entry);
    return entry;
}

list_node* list_insert_before(list_node *node, void *value) {
    list_node *entry = list_create_node();

    ensure_allocated(entry);

    entry->value = value;

    list_node *tmp = node->prev;
    node->prev = entry;
    entry->prev = tmp;
    entry->next = node;

    if (node->list != NULL) {
        node->list->size++;
    }

    return entry;
}

void list_remove(list_node *node) {
    node->prev = node->next;

    if (node->list != NULL) {
        node->list->size--;
    }

    free(node);
}
