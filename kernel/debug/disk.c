#include "console.h"

#include <kernel/disk/block.h>
#include <kernel/dispatch/events.h>

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

static void block_probe(tty_t* tty, const char* input) {
    block_device_t* device = block_device_get((char*) input);

    if (device == NULL) {
        tty_printf(tty, "Unknown block device: %s\n", input);
        return;
    }

    event_dispatch_async("block-device:partition-probe", device);
}

void debug_disk_init(void) {
    debug_register_command("block-list", block_list);
    debug_register_command("block-probe", block_probe);
}
