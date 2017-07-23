#include "filesystems.h"
#include "ext2.h"

void vfs_filesystems_init(void) {
    ext2_filesystem_init();
}
