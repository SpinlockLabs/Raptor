#include "console.h"

#include <kernel/device/registry.h>

static char* dev_classify(uint classifier) {
    switch (classifier) {
        case DEVICE_CLASS_BLOCK: return "block";
        case DEVICE_CLASS_INPUT: return "input";
        case DEVICE_CLASS_NETWORK: return "network";
        default: return "unknown";
    }
}

static void debug_device_list(tty_t* tty, const char* input) {
    unused(input);

    list_t* list = device_query(DEVICE_CLASS_ALL);

    list_for_each(node, list) {
        device_entry_t* device = node->value;
        tty_printf(
            tty,
            "Device [%s]:\n"
                "  Class: %s\n",
            device->name,
            dev_classify(device->classifier)
        );
    }
}

void debug_devices_init(void) {
    debug_console_register_command("device-list", debug_device_list);
}
