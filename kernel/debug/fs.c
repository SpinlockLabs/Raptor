#include "console.h"

#include <liblox/string.h>

#include <kernel/fs/vfs.h>

static void debug_fs_resolve(tty_t* tty, const char* input) {
    fs_node_t* node = fs_resolve((char*) input);
    if (node == NULL) {
        tty_printf(tty, "VFS failed to resolve: %s\n", input);
    } else {
        tty_printf(tty, "VFS resolved %s\n", input);
    }
}

static void debug_fs_mount(tty_t* tty, const char* input) {
    char* rest = strdup(input);

    uint i = 0;

    char* token;
    char* device = "";
    char* type = "";
    char* path = "";
    while ((token = strtok(rest, " ", &rest)) != NULL) {
        if (i == 0) {
            device = token;
        } else if (i == 1) {
            type = token;
        } else {
            path = token;
        }
        i++;
    }

    if (strcmp(device, "") == 0 || strcmp(type, "") == 0 || strcmp(path, "") == 0) {
        tty_printf(tty, "Usage: fs-mount <device> <fstype> <path>\n");
        free(rest);
        return;
    }

    block_device_t* dev = block_device_get(device);
    if (dev == NULL) {
        tty_printf(tty, "Unknown block device: %s\n", dev);
        free(rest);
        return;
    }

    fs_error_t error = vfs_mount(dev, type, path);

    if (error != FS_ERROR_OK) {
        tty_printf(tty, "Failed to mount device.\n");
    } else {
        tty_printf(tty, "Device mounted.\n");
    }

    free(rest);
}

static void debug_fs_cat(tty_t* tty, const char* input) {
    fs_node_t* node = fs_resolve((char*) input);
    if (node == NULL) {
        tty_printf(tty, "VFS failed to resolve: %s\n", input);
        return;
    }

    fs_stat_t stat;
    fs_error_t error = fs_stat(node, &stat);

    if (error != FS_ERROR_OK) {
        tty_printf(tty, "VFS failed to stat: %s\n", input);
        return;
    }

    uint8_t* buf = zalloc(stat.size);
    error = fs_read(node, 0, buf, stat.size);

    if (error != FS_ERROR_OK) {
        tty_printf(tty, "VFS failed to read: %s\n", input);
        free(buf);
        return;
    }
    buf[stat.size] = '\0';
    tty_write(tty, buf, stat.size);
    tty_printf(tty, "\n");
    free(buf);
}

static void debug_fs_list(tty_t* tty, const char* input) {
    fs_node_t* node = fs_resolve((char*) input);
    if (node == NULL) {
        tty_printf(tty, "VFS failed to resolve: %s\n", input);
        return;
    }

    fs_list_entry_t* entry = NULL;

    while (true) {
        fs_error_t error = fs_list(node, &entry);
        if (error != FS_ERROR_OK) {
            tty_printf(tty, "VFS failed to list: %s (%d)\n", input, error);
            return;
        }

        if (entry == NULL) {
            break;
        }

        tty_printf(tty, "%s\n", entry->name);
    }
}

static void debug_fs_stat(tty_t* tty, const char* input) {
    fs_node_t* node = fs_resolve((char*) input);
    if (node == NULL) {
        tty_printf(tty, "VFS failed to resolve: %s\n", input);
        return;
    }

    fs_stat_t stat;
    fs_error_t error = fs_stat(node, &stat);

    if (error != FS_ERROR_OK) {
        tty_printf(tty, "VFS failed to stat: %s\n", input);
        return;
    }

    tty_printf(tty, "Size: %d\n", stat.size);

    char* type = "unknown";

    if (stat.type == FS_TYPE_DIRECTORY) {
        type = "directory";
    } else if (stat.type == FS_TYPE_FILE) {
        type = "file";
    } else if (stat.type == FS_TYPE_LINK) {
        type = "link";
    }

    tty_printf(tty, "Type: %s\n", type);
}

void debug_fs_init(void) {
    debug_register_command("fs-resolve", debug_fs_resolve);
    debug_register_command("fs-stat", debug_fs_stat);
    debug_register_command("fs-mount", debug_fs_mount);
    debug_register_command("fs-cat", debug_fs_cat);
    debug_register_command("fs-list", debug_fs_list);
}
