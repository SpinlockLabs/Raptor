#include "rtl8139.h"
#include "dev.h"

#include <liblox/io.h>
#include <liblox/list.h>

#include <kernel/arch/x86/io.h>
#include <kernel/arch/x86/devices/pci/pci.h>
#include <kernel/spin.h>
#include <kernel/network/iface.h>
#include <liblox/string.h>
#include <kernel/arch/x86/arch-x86.h>

typedef struct rtl8139_iface {
    network_iface_t* iface;
    list_t* net_queue;
    uint8_t mac[6];
    int irq;
    uint32_t io_base;
    uint8_t* rx_buffer;
    uint8_t* tx_buffer[5];
    uintptr_t rx_phys;
    uintptr_t tx_phys[5];
    uint32_t current_rx;
    int dirty_tx;
    int next_tx;
    spin_lock_t net_queue_lock;
    spin_lock_t tx_buf_lock;
    uint8_t* last_packet;
} rtl8139_iface_t;

typedef struct rtl_net_buf {
    uint8_t* buffer;
    size_t size;
} rtl_net_buf_t;

static int next_tx_buf(rtl8139_iface_t* rtl) {
    int out;
    spin_lock(rtl->tx_buf_lock);
    out = rtl->next_tx;
    rtl->next_tx++;
    if (rtl->next_tx == 4) {
        rtl->next_tx = 0;
    }
    spin_unlock(rtl->tx_buf_lock);
    return out;
}

static void rtl_dequeue_task(void* extra) {
    rtl8139_iface_t* rtl = extra;

    spin_lock(rtl->net_queue_lock);
    if (rtl->net_queue->size == 0) {
        return;
    }

    list_node_t* node = list_dequeue(rtl->net_queue);
    rtl_net_buf_t* value = node->value;
    free(node);

    network_iface_t* iface = rtl->iface;

    if (iface->handle_receive != NULL) {
        iface->handle_receive(iface, value->buffer, value->size);
    }

    free(value);
    spin_unlock(rtl->net_queue_lock);
}

static void rtl_enqueue(rtl8139_iface_t* rtl, rtl_net_buf_t* buf) {
    spin_lock(rtl->net_queue_lock);
    list_add(rtl->net_queue, buf);
    spin_unlock(rtl->net_queue_lock);
}

static void rtl_do_send(rtl8139_iface_t* rtl, uint8_t* buffer, size_t size) {
    int tx = next_tx_buf(rtl);
    memcpy(rtl->tx_buffer[tx], buffer, size);
    outl((uint16_t) (rtl->io_base + RTL_PORT_TXBUF + 4 * tx), rtl->tx_phys[tx]);
    outl((uint16_t) (rtl->io_base + RTL_PORT_TXSTAT + 4 * tx), size);
}

static list_t* rtl_list = NULL;

static void rtl_device_interrupt(rtl8139_iface_t* rtl) {
    uint16_t status = ins((uint16_t) (rtl->io_base + RTL_PORT_ISR));

    if (!status) {
        return;
    }

    outs((uint16_t) (rtl->io_base + RTL_PORT_ISR), status);

    if (status & 0x01 || status & 0x02) {
        while ((inb((uint16_t) (rtl->io_base + RTL_PORT_CMD)) & 0x01) == 0) {
            int offset = rtl->current_rx % 0x2000;

            uint32_t* buf_start = (uint32_t*)((uintptr_t) rtl->rx_buffer + offset);
            uint32_t rx_status = buf_start[0];
            int rx_size = rx_status >> 16;

            if (!(rx_status & (0x0020 | 0x0010 | 0x0004 | 0x0002))) {
                uint8_t* bufd = (uint8_t*) &(buf_start[1]);

                rtl->last_packet = malloc((size_t) rx_size);

                uintptr_t packet_end = (uintptr_t) bufd + rx_size;

                if (packet_end > (uintptr_t) rtl->rx_buffer + 0x2000) {
                    size_t s = ((uintptr_t) rtl->rx_buffer + 0x2000) - (uintptr_t) bufd;
                    memcpy(rtl->last_packet, bufd, (size_t) rx_size);
                    memcpy(
                        (void*) ((uintptr_t) rtl->last_packet + s),
                        rtl->rx_buffer,
                        rx_size - s
                    );
                } else {
                    memcpy(rtl->last_packet, bufd, (size_t) rx_size);
                }

                rtl_net_buf_t* buf = zalloc(sizeof(rtl_net_buf_t));
                buf->buffer = rtl->last_packet;
                buf->size = (size_t) rx_size;
                rtl_enqueue(rtl, buf);
            }
        }
    }

    if (status & 0x08 || status & 0x04) {
        uint i = inl((uint16_t) (rtl->io_base + RTL_PORT_TXSTAT + 4 * rtl->dirty_tx));
        unused(i);
        rtl->dirty_tx++;
        if (rtl->dirty_tx == 5) {
            rtl->dirty_tx = 0;
        }
    }
}

static int rtl_irq_handler(cpu_registers_t* r) {
    unused(r);

    list_for_each(node, rtl_list) {
        rtl8139_iface_t* iface = node->value;
        rtl_device_interrupt(iface);
    }
    return 0;
}

static void rtl8139_init(uint32_t did) {
    unused(did);
}

static void find_rtl8139(uint32_t device, uint16_t vendor_id, uint16_t device_id, void* extra) {
    unused(extra);

    if ((vendor_id == 0x10EC) && (device_id == 0x8139)) {
        rtl8139_init(device);
    }
}

void rtl8139_setup(void) {
    rtl_list = list_create();
    pci_scan(&find_rtl8139, -1, NULL);
}
