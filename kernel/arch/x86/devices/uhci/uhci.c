#include "uhci.h"
#include "defs.h"

#include <liblox/io.h>
#include <liblox/memory.h>
#include <liblox/string.h>

#include <kernel/device/registry.h>
#include <kernel/usb/controller.h>

#include <kernel/usb/device.h>

#include <kernel/arch/x86/devices/pci/pci.h>
#include <kernel/arch/x86/io.h>
#include <liblox/sleep.h>

#define PCI_SERIAL_USB 0x0c03


typedef struct uhci {
    usb_controller_t* controller;
    UhciQueueHead* queue_head_pool;
    UhciTransferDescriptor* descriptor_pool;
    uint32_t* frames;
    uint16_t io_addr;
    UhciQueueHead* async_queue_head;
} uhci_t;

static UhciQueueHead* uhci_alloc_queue_head(uhci_t* uhci) {
    UhciQueueHead* end = uhci->queue_head_pool + MAX_QH;
    for (UhciQueueHead* qh = uhci->queue_head_pool; qh != end; ++qh) {
        if (!qh->active) {
            qh->active = 1;
            return qh;
        }
    }
    return NULL;
}

static void uhci_poll(usb_controller_t* controller) {

}

static void uhci_port_set(uint16_t addr, uint16_t value) {
    uint16_t status = ins(addr);
    status |= value;
    status &= ~PORT_RWC;
    outs(addr, status);
}

static void uhci_port_clr(uint16_t addr, uint16_t value) {
    uint16_t status = ins(addr);
    status &= ~PORT_RWC;
    status &= ~value;
    outs(addr, status);
}

static uint32_t uhci_reset_port(uhci_t* uhci, uint port) {
    uint32_t reg = REG_PORT1 + port * 2;
    uhci_port_set((uint16_t) (uhci->io_addr + reg), PORT_RESET);
    sleep(1);
    uhci_port_clr((uint16_t) (uhci->io_addr + reg), PORT_RESET);

    uint32_t status = 0;
    for (uint i = 0; i < 10; ++i) {
        sleep(1);

        status = ins((uint16_t) (uhci->io_addr + reg));

        if (~status & PORT_CONNECTION) {
            break;
        }

        if (status & (PORT_ENABLE_CHANGE | PORT_CONNECTION_CHANGE)) {
            uhci_port_clr(
                (uint16_t) (uhci->io_addr + reg),
                PORT_ENABLE_CHANGE | PORT_CONNECTION_CHANGE
            );
            continue;
        }

        if (status & PORT_ENABLE) {
            break;
        }

        uhci_port_set((uint16_t) (uhci->io_addr + reg), PORT_ENABLE);
    }
    return status;
}

static void uhci_probe(uhci_t* uhci) {
    uint port_count = 2;
    for (uint port = 0; port < port_count; ++port) {
        uint32_t status = uhci_reset_port(uhci, port);

        if (!(status & PORT_ENABLE)) {
            continue;
        }

        printf(DEBUG "Got a USB device.\n");
    }
}

static void uhci_setup_pci(device_entry_t* parent) {
    pci_device_t* device = parent->device;

    if (pci_find_type(device->device_id) != PCI_SERIAL_USB) {
        return;
    }

    printf(DEBUG "Using '%s' as a UHCI USB controller.\n", device->name);

    usb_controller_t* controller = zalloc(sizeof(usb_controller_t));
    uhci_t* uhci = zalloc(sizeof(uhci_t));
    uhci->controller = controller;
    strcpy(controller->name, "usb-controller");
    controller->ops.poll = uhci_poll;
    controller->internal.data = uhci;

    uhci->io_addr = (uint16_t) (
        pci_read_field(device->device_id, PCI_BAR0, 4)
            & 0xFFFFFFF0
    );

    uhci->frames = zalloc(1024 * sizeof(uint32_t));
    uhci->queue_head_pool = zalloc(sizeof(UhciQueueHead) * MAX_QH);
    uhci->descriptor_pool = zalloc(sizeof(UhciTransferDescriptor) * MAX_TD);

    UhciQueueHead* qh = uhci_alloc_queue_head(uhci);
    qh->head = TD_PTR_TERMINATE;
    qh->element = TD_PTR_TERMINATE;
    qh->transfer = 0;

    uhci->async_queue_head = qh;

    for (uint i = 0; i < 1024; ++i) {
        uhci->frames[i] = TD_PTR_QH | (uint32_t) (uintptr_t) qh;
    }

    outs((uint16_t) (uhci->io_addr + REG_LEGSUP), 0x8f00);
    outs((uint16_t) (uhci->io_addr + REG_INTR), 0);

    outs((uint16_t) (uhci->io_addr + REG_FRNUM), 0);
    outl((uint16_t) (uhci->io_addr + REG_FRBASEADD), (uint32_t) (uintptr_t) uhci->frames);
    outs((uint16_t) (uhci->io_addr + REG_SOFMOD), 0x40);

    outs((uint16_t) (uhci->io_addr + REG_STS), 0xffff);
    outs((uint16_t) (uhci->io_addr + REG_CMD), CMD_RS);

    uhci_probe(uhci);

    controller->entry = device_register(
        parent,
        "usb-controller",
        DEVICE_CLASS_USB_CONTROLLER,
        controller
    );
}

void uhci_setup(void) {
    list_t* list = device_query(DEVICE_CLASS_PCI);
    list_for_each(node, list) {
        uhci_setup_pci(node->value);
    }
    list_free(list);
}
