#include <liblox/memory.h>
#include <liblox/string.h>
#include <liblox/io.h>
#include "embdfs.h"

typedef struct embdfs_entry {
    char* name;
    char* data;
    size_t size;
} embdfs_entry_t;

static embdfs_entry_t default_entries[] = {
    {"hello", "Hello World!", 12},
    {NULL, NULL, 0}
};

static fs_error_t embdfs_list(fs_node_t* node, fs_list_entry_t** eout) {
    unused(node);

    embdfs_entry_t* last = &default_entries[0];
    if (*eout != NULL) {
        last = (*eout)->internal.tag;
        free(*eout);
        *eout = NULL;
        last++;
    }

    if (last->name == NULL) {
        *eout = NULL;
        return FS_ERROR_OK;
    }

    if (last->name == NULL) {
        *eout = NULL;
        return FS_ERROR_OK;
    }

    fs_list_entry_t* entry = zalloc(sizeof(fs_list_entry_t));
    memcpy(entry->name, last->name, strlen(last->name));
    entry->internal.tag = last;
    *eout = entry;
    return FS_ERROR_OK;
}

static fs_error_t embdfs_stat(fs_node_t* node, fs_stat_t* stat) {
    embdfs_entry_t* entry = node->internal.tag;

    if (entry == NULL) {
        stat->size = 0;
        stat->type = FS_TYPE_DIRECTORY;
        return FS_ERROR_OK;
    }

    stat->size = entry->size;
    stat->type = FS_TYPE_FILE;
    return FS_ERROR_OK;
}

static fs_error_t embdfs_read(fs_node_t* node, size_t offset, uint8_t* buffer, size_t size) {
    embdfs_entry_t* entry = node->internal.tag;

    if (entry == NULL) {
        return FS_ERROR_BAD_TYPE;
    }

    if (entry->size - offset > size) {
        return FS_ERROR_TOO_BIG;
    }

    memcpy(buffer, entry->data + offset, size);
    return FS_ERROR_OK;
}

static fs_error_t embdfs_child(fs_node_t* node, char* name, fs_node_t** out) {
    unused(node);

    for (embdfs_entry_t* entry = &default_entries[0]; entry->name != NULL; entry++) {
        if (strcmp(name, entry->name) != 0) {
            continue;
        }

        fs_node_t* child = fs_create_node(entry->name);
        child->read = embdfs_read;
        child->stat = embdfs_stat;
        child->internal.tag = entry;
        *out = child;
        return FS_ERROR_OK;
    }

    return FS_ERROR_DOES_NOT_EXIST;
}

fs_node_t* embdfs_mount(block_device_t* block) {
    unused(block);

    printf(DEBUG "[embdfs] Mounting via block '%s'\n", block->name);

    fs_node_t* node = fs_create_node("/");
    node->list = embdfs_list;
    node->child = embdfs_child;
    node->stat = embdfs_stat;
    return node;
}

void embdfs_init(void) {
    vfs_register_filesystem("embdfs", embdfs_mount);
}
