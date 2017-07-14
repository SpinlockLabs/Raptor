#include "console.h"

#include <kernel/disk/block.h>

static void block_list(tty_t* tty, const char* input) {
    unused(input);

    list_t* disks = block_device_get_all();

    list_for_each(node, disks) {
        block_device_t* device = node->value;

        tty_printf(tty, "- Name: %s\n", device->name);
        block_device_stat_t stat = {0};
        if (block_device_stat(
            device,
            &stat
        ) != BLOCK_DEVICE_ERROR_OK) {
            tty_printf(tty, "  (Stat Failed)\n", device);
        } else {
            tty_printf(tty, "  Size: %d bytes\n", stat.size);
        }
    }
}

void debug_disk_init(void) {
    debug_console_register_command("block-list", block_list);
}
