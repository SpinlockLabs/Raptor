#include "registry.h"

#include <liblox/io.h>
#include <liblox/string.h>

#include <kernel/spin.h>
#include <kernel/dispatch/events.h>

static tree_t* device_tree = NULL;
static spin_lock_t lock;

device_entry_t* device_root(void) {
    if (device_tree == NULL || device_tree->root == NULL) {
        return NULL;
    }

    return device_tree->root->value;
}

device_entry_t* device_register(
    device_entry_t* parent,
    char* name,
    uint classifier,
    void* device
) {
    if (parent == NULL || name == NULL || device == NULL) {
        return NULL;
    }

    device_entry_t* existing = device_lookup(
        parent,
        name,
        classifier
    );

    if (existing != NULL) {
        printf(WARN "Device %s already exists.\n", name);
    }

    device_entry_t* entry = zalloc(sizeof(device_entry_t));
    entry->name = name;
    entry->classifier = classifier;
    entry->device = device;

    spin_lock(&lock);
    tree_node_t* node = tree_node_insert_child(
        device_tree,
        parent->node,
        entry
    );
    entry->node = node;
    spin_unlock(&lock);

    event_dispatch_async(EVENT_DEVICE_REGISTERED, entry);

    return entry;
}

bool device_unregister(
    device_entry_t* device
) {
    if (device == NULL) {
        return false;
    }

    tree_node_t* node = device->node;
    if (node != NULL) {
        event_dispatch(EVENT_DEVICE_UNREGISTERED, node->value);

        spin_lock(&lock);
        free(node->value);
        tree_node_remove(device_tree, node);
        spin_unlock(&lock);
    }

    return node != NULL;
}

static void do_device_query(
    device_entry_t* parent,
    list_t* list,
    device_class_t classifier
) {
    if (parent == NULL) {
        return;
    }

    if (classifier == 0 || parent->classifier == classifier) {
        list_add(list, parent);
    }

    list_for_each(lnode, parent->node->children) {
        tree_node_t* node = lnode->value;
        device_entry_t* entry = node->value;
        do_device_query(entry, list, classifier);
    }
}

list_t* device_query(device_class_t classifier) {
    list_t* list = list_create();

    spin_lock(&lock);
    do_device_query(
        device_root(),
        list,
        classifier
    );
    spin_unlock(&lock);

    return list;
}

static device_entry_t* do_device_lookup(
    device_entry_t* parent,
    char* name,
    device_class_t classifier
) {
    if (parent == NULL || parent->node == NULL) {
        return NULL;
    }

    if ((classifier == 0 || parent->classifier == classifier) &&
        strcmp(parent->name, name) == 0) {
        return parent;
    }

    list_for_each(lnode, parent->node->children) {
        tree_node_t* node = lnode->value;
        device_entry_t* entry = node->value;
        device_entry_t* result = do_device_lookup(
            entry,
            name,
            classifier
        );

        if (result != NULL) {
            return result;
        }
    }

    return NULL;
}

device_entry_t* device_lookup(
    device_entry_t* parent,
    char* name,
    device_class_t classifier
) {
    spin_lock(&lock);
    device_entry_t* result = do_device_lookup(
        parent,
        name,
        classifier
    );
    spin_unlock(&lock);
    return result;
}

void device_registry_init(void) {
    device_tree = tree_create();
    spin_init(&lock);

    device_entry_t* entry = zalloc(sizeof(device_entry_t));
    entry->name = "root";
    entry->classifier = DEVICE_CLASS_ROOT;
    tree_set_root(device_tree, entry);
    entry->node = device_tree->root;
}
