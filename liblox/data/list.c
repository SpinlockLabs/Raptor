#include "../common.h"
#include "../list.h"
#include "../string.h"

#define list_invalid_index(list, idx) (idx) >= (list)->size

void list_init_node(list_node_t* node) {
    memset(node, 0, sizeof(list_node_t));
}

void list_init(list_t* list) {
    memset(list, 0, sizeof(list_t));
    list->free_values = true;
}

list_node_t* list_create_node(void) {
    list_node_t* node = (list_node_t*) zalloc(sizeof(list_node_t));

    ensure_allocated(node);

    list_init_node(node);
    return node;
}

list_t* list_create(void) {
    list_t* val = zalloc(sizeof(list_t));

    ensure_allocated(val);

    list_init(val);
    return val;
}

list_node_t* list_insert_node_after(list_node_t* node, list_node_t* entry) {
    ensure_allocated(entry);
    ensure_allocated(entry->list);

    if (node == NULL) {
        entry->list->head = entry;
        entry->list->tail = entry;
        entry->list->size = 1;
        return entry;
    }

    list_node_t* tmp = node->next;
    node->next = entry;
    entry->prev = node;
    entry->next = tmp;

    if (node->list != NULL) {
        node->list->size++;

        if (node->list->tail == node) {
            node->list->tail = node;
        }
    }

    return entry;
}

list_node_t* list_insert_after(list_node_t* node, void* value) {
    list_node_t* entry = list_create_node();
    ensure_allocated(entry);
    entry->list = node->list;
    entry->value = value;

    return list_insert_node_after(node, entry);
}

list_node_t* list_insert_node_before(list_node_t* node, list_node_t* entry) {
    ensure_allocated(entry);
    ensure_allocated(entry->list);

    if (node == NULL) {
        entry->list->head = entry;
        entry->list->tail = entry;
        entry->list->size = 1;
        return entry;
    }

    list_node_t* tmp = node->prev;
    node->prev = entry;
    entry->prev = tmp;
    entry->next = node;

    if (node == entry->list->head) {
        entry->list->head = entry;
    }

    if (node->list != NULL) {
        node->list->size++;
    }

    return entry;
}

list_node_t* list_insert_before(list_node_t* node, void* value) {
    list_node_t* entry = list_create_node();
    ensure_allocated(entry);
    entry->list = node->list;
    entry->value = value;

    return list_insert_node_before(node, entry);
}

list_node_t* list_add_node(list_t* list, list_node_t* entry) {
    ensure_allocated(list);
    ensure_allocated(entry);

    if (list->head == NULL) {
        list->head = entry;
        list->size = 1;
        return entry;
    }

    list_node_t* current = list->head;
    while (current->next != NULL) {
        current = current->next;
    }
    list_insert_node_after(current, entry);
    return entry;
}

list_node_t* list_add(list_t* list, void* value) {
    list_node_t* entry = list_create_node();
    ensure_allocated(entry);
    entry->list = list;
    entry->value = value;

    return list_add_node(list, entry);
}

list_node_t* list_get_at(list_t* list, size_t index) {
    if (list_invalid_index(list, index)) {
        return NULL;
    }

    size_t current = 0;
    list_node_t* node = list->head;
    while (node != NULL && current != index && current < list->size) {
        node = node->next;
        current++;
    }

    return node;
}

void* list_get_value_at(list_t* list, size_t index) {
    list_node_t* node = list_get_at(list, index);

    if (node == NULL) {
        return NULL;
    }

    return node->value;
}

list_node_t* list_find(list_t* list, void* value) {
    if (list == NULL) {
        return NULL;
    }

    list_node_t* node = list->head;

    while (node != NULL) {
        if (node->value == value) {
            return node->value;
        }

        node = node->next;
    }

    return NULL;
}

void list_remove(list_node_t* node) {
    if (node == NULL) {
        return;
    }

    list_node_t* prev = node->prev;
    list_node_t* next = node->next;

    if (prev != NULL) {
        prev->next = next;
    }

    if (next != NULL) {
        next->prev = prev;
    }

    if (node->list != NULL && node->list->head == node) {
        node->list->head = next;
    }

    if (node->list != NULL && node->list->tail == node) {
        node->list->tail = NULL;
    }

    if (node->list != NULL) {
        node->list->size--;
    }
}

void list_merge(list_t* target, list_t* source) {
    /* Destructively merges source into target */
    list_for_each(node, source) {
        node->list = target;
    }

    if (source->head) {
        source->head->prev = target->tail;
    }

    if (target->tail) {
        target->tail->next = source->head;
    } else {
        target->head = source->head;
    }

    if (source->tail) {
        target->tail = source->tail;
    }
    target->size += source->size;
    free(source);
}

bool list_contains(list_t* list, void* value) {
    return list_find(list, value) != NULL;
}

list_t* list_diff(list_t* left, list_t* right) {
    if (left == NULL || right == NULL) {
        return NULL;
    }

    list_t* output = list_create();
    list_for_each(node, left) {
        bool exists = list_contains(right, node->value);

        if (!exists) {
            list_add(output, node->value);
        }
    }
    return output;
}

void list_free_entries(list_t* list) {
    list_node_t* node = list->head;

    while (node != NULL) {
        list_node_t* next = node->next;

        free(node);

        node = next;
    }
}

void list_free(list_t* list) {
    list_node_t* node = list->head;

    while (node != NULL) {
        list_node_t* next = node->next;

        if (list->free_values && node->value != NULL) {
            free(node->value);
        }
        free(node);

        node = next;
    }

    free(list);
}

list_node_t* list_dequeue(list_t* list) {
    if (list->head == NULL) {
        return NULL;
    }

    list_node_t* out = list->head;
    list_remove(out);
    return out;
}

void list_swap(list_node_t* left, list_node_t* right) {
    void* tmp = left->value;
    left->value = right->value;
    right->value = tmp;
}

static void list_bubble_sort(list_t* list, list_compare_t compare) {
    size_t size = list->size;
    bool swapped;

    do {
        swapped = false;

        for (size_t index = 1; index < size - 1; index++) {
            list_node_t* left = list_get_at(list, index - 1);
            list_node_t* right = list_get_at(list, index);

            if (compare(left->value, right->value) > 0) {
                list_swap(left, right);
                swapped = true;
            }
        }
    } while (swapped);
}

void list_sort(list_t* list, list_compare_t compare) {
    list_bubble_sort(list, compare);
}
