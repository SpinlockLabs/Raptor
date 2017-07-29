#include "vfs.h"

#include <liblox/io.h>
#include <liblox/string.h>
#include <liblox/hashmap.h>
#include <liblox/tree.h>

#include <kernel/spin.h>

#include "filesystems.h"

static fs_node_t* fs_root;
static vfs_entry_t* vfs_root;
static tree_t* tree = NULL;
static spin_lock_t lock;
static hashmap_t* filesystems;

static void vfs_dump(tree_node_t* c, uint level) {
    list_for_each(a, c->children) {
        tree_node_t* at = a->value;
        vfs_entry_t* ent = at->value;
        printf("N: %s\n", ent->name);
        printf("L: %d\n", level);
        vfs_dump(at, level + 1);
    }
}

used static void vfs_dump_tree(tree_node_t* node) {
    printf("Tree at 0x%x\n", node);
    vfs_dump(node, 0);
}

fs_node_t* fs_create_node(char* name) {
    size_t name_size = strlen(name);

    if (name_size > VFS_MAX_NAME_SIZE - 1) {
        name_size = VFS_MAX_NAME_SIZE - 1;
    }

    fs_node_t* node = zalloc(sizeof(fs_node_t));

    memcpy(&node->name, name, name_size);
    return node;
}

vfs_entry_t* vfs_create_entry(char* name) {
    vfs_entry_t* entry = zalloc(sizeof(vfs_entry_t));
    entry->name = name;
    return entry;
}

void vfs_subsystem_init(void) {
    filesystems = hashmap_create(2);
    tree = tree_create();

    fs_root = fs_create_node("[root]");
    vfs_root = vfs_create_entry("[root]");
    vfs_root->fs = fs_root;
    tree_set_root(tree, vfs_root);
    vfs_filesystems_init();
}

fs_error_t vfs_register_filesystem(char* name, vfs_filesystem_mounter_t mounter) {
    if (hashmap_has(filesystems, name)) {
        return FS_ERROR_EXISTS;
    }

    hashmap_set(filesystems, name, mounter);
    return FS_ERROR_OK;
}

fs_error_t vfs_mount(block_device_t* block, char* type, char* path) {
    if (block == NULL) {
        return FS_ERROR_BAD_CALL;
    }

    if (!hashmap_has(filesystems, type)) {
        return FS_ERROR_DOES_NOT_EXIST;
    }

    vfs_filesystem_mounter_t mounter = hashmap_get(filesystems, type);
    fs_node_t* node = mounter(block);
    if (node == NULL) {
        return FS_ERROR_BAD_STATE;
    }

    fs_error_t error = vfs_mount_node(path, node);
    return error;
}

fs_error_t vfs_mount_node(char* path, fs_node_t* node) {
    if (!tree) {
        return FS_ERROR_BAD_STATE;
    }

    spin_lock(lock);
    tree_node_t* tnode = NULL;

    char* p = strdup(path);
    char* i = p;

    int path_size = strlen(p);

    while (i < p + path_size) {
        if (*i == VFS_PATH_SEP) {
            *i = '\0';
        }
        i++;
    }
    p[path_size] = '\0';
    i = p + 1;

    if (*i == '\0') {
        vfs_entry_t* r = vfs_root;
        r->fs = node;
        fs_root = node;
        free(p);
        spin_unlock(lock);
        return FS_ERROR_OK;
    }

    tnode = tree->root;

    char* at = i;
    while (true) {
        if (at >= p + path_size) {
            break;
        }

        bool found = false;

        list_for_each(child, tnode->children) {
            tree_node_t* tchild = child->value;
            vfs_entry_t* entry = tchild->value;
            if (strcmp(entry->name, at) == 0) {
                found = true;
                tnode = tchild;
                break;
            }
        }

        if (!found) {
            vfs_entry_t* entry = vfs_create_entry(strdup(at));
            tnode = tree_node_insert_child(tree, tnode, entry);
        }

        at = at + strlen(at) + 1;
    }

    vfs_entry_t* entry = tnode->value;
    if (entry->fs) {
        printf(
            WARN "VFS is mounting a filesystem on"
                 " '%s' where one already exists.\n",
            path
        );
    }

    entry->fs = node;

    free(p);
    spin_unlock(lock);
    return FS_ERROR_OK;
}

fs_error_t fs_get_child(fs_node_t* parent, char* child, fs_node_t** node) {
    *node = NULL;

    if (parent == NULL) {
        return FS_ERROR_BAD_CALL;
    }

    if (child == NULL) {
        *node = parent;
        return FS_ERROR_OK;
    }

    if (parent->child == NULL) {
        return FS_ERROR_NOT_IMPLEMENTED;
    }

    return parent->child(parent, child, node);
}

static fs_node_t* vfs_get_mount(
    char* path,
    uint path_depth,
    char** out_path,
    uint* out_depth
) {
    size_t depth;

    for (depth = 0; depth <= path_depth; ++depth) {
        path += strlen(path) + 1;
    }

    fs_node_t* last = fs_root;
    tree_node_t* node = tree->root;

    char* at = *out_path;
    uint _depth = 1;
    uint _tree_depth = 0;

    while (true) {
        if (at >= path) {
            break;
        }

        bool found = false;

        list_for_each(child, node->children) {
            tree_node_t* tchild = child->value;
            vfs_entry_t* entry = tchild->value;

            if (strcmp(entry->name, at) == 0) {
                found = true;
                node = tchild;
                at = at + strlen(at) + 1;

                if (entry->fs) {
                    _tree_depth = _depth;
                    last = entry->fs;
                    *out_path = at;
                }
                break;
            }
        }

        if (!found) {
            break;
        }

        _depth++;
    }

    *out_depth = _tree_depth;

    return last;
}

fs_node_t* fs_resolve(char* _path) {
    if (_path == NULL) {
        return NULL;
    }

    char* path = strdup(_path);
    size_t path_size = strlen(path);

    if (path_size == 1 && strcmp("/", path) == 0) {
        free(path);
        return fs_root;
    }

    char* path_offset = path;
    uint32_t path_depth = 0;

    while (path_offset < path + path_size) {
        if (*path_offset == VFS_PATH_SEP) {
            *path_offset = '\0';
            path_depth++;
        }

        path_offset++;
    }

    path[path_size] = '\0';
    path_offset = path + 1;

    uint depth = 0;

    fs_node_t* mount = vfs_get_mount(path, path_depth, &path_offset, &depth);

    if (mount == NULL) {
        free(path);
        return NULL;
    }

    if (path_offset >= path + path_size) {
        free(path);
        return mount;
    }

    fs_node_t* node = mount;
    fs_error_t error;

    for (; depth < path_depth; ++depth) {
        error = fs_get_child(node, path_offset, &node);
        if (error != FS_ERROR_OK || node == NULL) {
            free(path);
            return NULL;
        }

        if (depth == path_depth - 1) {
            free(path);
            return node;
        }

        path_offset += strlen(path_offset) + 1;
    }

    free(path);
    return NULL;
}

fs_error_t fs_read(fs_node_t* node, size_t offset, uint8_t* buffer, size_t size) {
    if (node == NULL) {
        return FS_ERROR_BAD_CALL;
    }

    if (node->read == NULL) {
        return FS_ERROR_NOT_IMPLEMENTED;
    }

    fs_error_t error = node->read(node, offset, buffer, size);

    return error;
}

fs_error_t fs_stat(fs_node_t* node, fs_stat_t* stat) {
    if (node == NULL) {
        return FS_ERROR_BAD_CALL;
    }

    if (node->stat == NULL) {
        return FS_ERROR_NOT_IMPLEMENTED;
    }

    fs_error_t error = node->stat(node, stat);

    return error;
}

fs_error_t fs_list(fs_node_t* node, fs_list_entry_t** entry) {
    if (node == NULL) {
        return FS_ERROR_BAD_CALL;
    }

    if (node->list == NULL) {
        return FS_ERROR_NOT_IMPLEMENTED;
    }

    fs_error_t error = node->list(node, entry);
    return error;
}
