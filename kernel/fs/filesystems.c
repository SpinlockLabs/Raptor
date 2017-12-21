#include "filesystems.h"
#include "ext2.h"
#include "embdfs.h"

void vfs_filesystems_init(void) {
    ext2_filesystem_init();
    embdfs_init();
}
