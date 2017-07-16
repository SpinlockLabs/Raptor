#include "console.h"

#include <kernel/fs/vfs.h>

static void debug_vfs_resolve(tty_t* tty, const char* input) {
    fs_node_t* node = vfs_resolve((char*) input);
    if (node == NULL) {
        tty_printf(tty, "VFS failed to resolve: %s\n", input);
    } else {
        tty_printf(tty, "VFS resolved %s\n", input);
    }
}

void debug_fs_init(void) {
    debug_console_register_command("vfs-resolve", debug_vfs_resolve);
}
