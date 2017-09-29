#include "console.h"

#include <kernel/device/registry.h>

static char* dev_classify(uint classifier) {
    switch (classifier) {
        case DEVICE_CLASS_BLOCK: return "block";
        case DEVICE_CLASS_INPUT: return "input";
        case DEVICE_CLASS_NETWORK: return "network";
        case DEVICE_CLASS_PCI: return "pci";
        case DEVICE_CLASS_ROOT: return "root";
        default: return "unknown";
    }
}

static void debug_device_list(tty_t* tty, const char* input) {
    unused(input);

    list_t* list = device_query(0);

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
    debug_register_command((console_command_t) {
        .name = "device-list",
        .group = "devices",
        .help = "List all current devices",
        .cmd = debug_device_list
    });
}
