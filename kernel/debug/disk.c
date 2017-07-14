#include "console.h"

#include <kernel/disk/disk.h>

static void disk_list(tty_t* tty, const char* input) {
    list_t* disks = block_device_get_all();

    list_for_each(node, disks) {
        block_device_t* device = node->value;

        tty_printf(tty, "- Name: %s\n", device->name);
        block_device_stat_t stat;
        block_device_stat(
            device,
            &stat
        );

        tty_printf(tty, "  Size: %d bytes\n", device);
    }
}

void debug_disk_init(void) {
    debug_console_register_command("disk-list", disk_list);
}
