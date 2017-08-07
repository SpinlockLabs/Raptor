#include "rootfs.h"

#include <liblox/string.h>
#include <liblox/memory.h>
#include <liblox/io.h>

#include <kernel/cmdline.h>

#include <kernel/disk/block.h>
#include <kernel/fs/vfs.h>

void mount_rootfs(void) {
    char* root = cmdline_read("root");
    if (root == NULL) {
        return;
    }

    char* root_type = cmdline_read("root.type");

    if (root_type == NULL) {
        root_type = strdup("ext2");
    }

    printf(
        INFO "Mounting %s (%s) as the root filesystem...\n",
        root,
        root_type
    );

    block_device_t* device = block_device_get(root);
    fs_error_t error = vfs_mount(device, root_type, "/");
    if (error != FS_ERROR_OK) {
        printf(ERROR "Failed to mount root filesystem: %d\n", error);
    }

    free(root);
    free(root_type);

    printf(
      INFO "Root filesystem mounted.\n"
    );
}
