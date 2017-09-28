#include "e1000.h"

#include "devices.h"
#include "defs.h"

#include <liblox/common.h>
#include <liblox/list.h>
#include <liblox/string.h>
#include <liblox/sleep.h>
#include <liblox/printf.h>
#include <liblox/memory.h>

#include <kernel/spin.h>
#include <kernel/cpu/task.h>
#include <kernel/network/iface.h>

#include <kernel/arch/x86/irq.h>
#include <kernel/arch/x86/paging.h>
#include <kernel/arch/x86/heap.h>
#include <kernel/arch/x86/devices/pci/pci.h>

static uint32_t mmio_read32(uintptr_t addr) {
    return *((volatile uint32_t*) (addr));
}

static void mmio_write32(uintptr_t addr, uint32_t val) {
    (*((volatile uint32_t*) (addr))) = val;
}

struct rx_desc {
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint16_t checksum;
    volatile uint8_t status;
    volatile uint8_t errors;
    volatile uint16_t special;
} packed;

struct tx_desc {
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint8_t cso;
    volatile uint8_t cmd;
    volatile uint8_t status;
    volatile uint8_t css;
    volatile uint16_t special;
} packed;

typedef struct e1000_state {
    uint32_t device_pci;
    size_t irq;
    uintptr_t mem_base;
    bool has_eeprom;
    uint8_t mac[6];
    int rx_index;
    int tx_index;
    spin_lock_t net_queue_lock;
    list_t* net_queue;
    uint8_t* rx_virt[E1000_NUM_RX_DESC];
    uint8_t* tx_virt[E1000_NUM_TX_DESC];
    struct rx_desc* rx;
    struct tx_desc* tx;
    uintptr_t rx_phys;
    uintptr_t tx_phys;
} e1000_state_t;

typedef struct e1000_iface {
    netif_t* iface;
    e1000_state_t* state;
    ktask_id poll_task;
} e1000_iface_t;

typedef struct e1000_netbuf {
    uint8_t* buffer;
    size_t size;
} e1000_netbuf_t;

static void write_command(e1000_state_t* state, uint16_t addr, uint32_t val) {
    mmio_write32(state->mem_base + addr, val);
}

static uint32_t read_command(e1000_state_t* state, uint16_t addr) {
    return mmio_read32(state->mem_base + addr);
}

static void enqueue_packet(e1000_state_t* state, e1000_netbuf_t* buffer) {
    spin_lock(&state->net_queue_lock);
    list_add(state->net_queue, buffer);
    spin_unlock(&state->net_queue_lock);
}

static void dequeue_packet_task(void* data) {
    e1000_iface_t* net = data;
    e1000_state_t* state = net->state;

    spin_lock(&state->net_queue_lock);
    if (state->net_queue->size == 0) {
        spin_unlock(&state->net_queue_lock);
        return;
    }

    list_node_t* n = list_dequeue(state->net_queue);
    e1000_netbuf_t* value = n->value;
    free(n);
    spin_unlock(&state->net_queue_lock);

    netif_t* iface = net->iface;
    if (iface->handle_receive != NULL) {
        iface->handle_receive(iface, value->buffer, value->size);
    }
    free(value);
}

static int eeprom_detect(e1000_state_t* state) {
    write_command(state, E1000_REG_EEPROM, 1);

    for (int i = 0; i < 100000 && !state->has_eeprom; ++i) {
        uint32_t val = read_command(state, E1000_REG_EEPROM);
        if (val & 0x10) {
            state->has_eeprom = true;
        }
    }

    return 0;
}

static uint16_t eeprom_read(e1000_state_t* state, uint8_t addr) {
    uint32_t temp = 0;
    write_command(state, E1000_REG_EEPROM, 1 | ((uint32_t) (addr) << 8));
    while (!((temp = read_command(state, E1000_REG_EEPROM)) & (1 << 4))) {}
    return (uint16_t) ((temp >> 16) & 0xFFFF);
}

static void enable_promisc(e1000_state_t* state) {
    uint32_t rctl = read_command(state, E1000_REG_RCTRL);

    rctl |= RCTL_UPE;
    rctl |= RCTL_MPE;

    write_command(
        state,
        E1000_REG_RCTRL,
        rctl
    );
}

static void disable_promisc(e1000_state_t* state) {
    uint32_t rctl = read_command(state, E1000_REG_RCTRL);

    rctl &= ~(RCTL_UPE);
    rctl &= ~(RCTL_MPE);

    write_command(
        state,
        E1000_REG_RCTRL,
        rctl
    );
}

static bool get_promisc(e1000_state_t* state) {
    uint32_t rctl = read_command(state, E1000_REG_RCTRL);

    if ((rctl >> 3) & 1) {
        return true;
    }

    return false;
}

static int e1000_ioctl(netif_t* iface, ulong req, void* data) {
    unused(data);

    e1000_iface_t* net = iface->data;

    if (req == NET_IFACE_IOCTL_ENABLE_PROMISCUOUS) {
        enable_promisc(net->state);
        return 0;
    }

    if (req == NET_IFACE_IOCTL_DISABLE_PROMISCUOUS) {
        disable_promisc(net->state);
        return 0;
    }

    if (req == NET_IFACE_IOCTL_GET_PROMISCUOUS) {
        return get_promisc(net->state);
    }

    return -1;
}

static void read_mac(e1000_state_t* state) {
    if (state->has_eeprom) {
        uint32_t t;
        t = eeprom_read(state, 0);
        state->mac[0] = (uint8_t) (t & 0xFF);
        state->mac[1] = (uint8_t) (t >> 8);
        t = eeprom_read(state, 1);
        state->mac[2] = (uint8_t) (t & 0xFF);
        state->mac[3] = (uint8_t) (t >> 8);
        t = eeprom_read(state, 2);
        state->mac[4] = (uint8_t) (t & 0xFF);
        state->mac[5] = (uint8_t) (t >> 8);
    } else {
        uint8_t* mac_addr = (uint8_t*) (state->mem_base + 0x5400);
        for (int i = 0; i < 6; ++i) {
            state->mac[i] = mac_addr[i];
        }
    }
}

static list_t* e1000_list = NULL;

static int e1000_device_interrupt(e1000_iface_t* iface) {
    e1000_state_t* state = iface->state;
    uint32_t status = read_command(state, 0xc0);

    if (!status) {
        return 0;
    }

    if (status & 0x04) {
        /* Start link. */
    } else if (status & 0x10) {
        /* ?? */
    } else if (status & ((1 << 6) | (1 << 7))) {
        /* Packet received. */
        do {
            state->rx_index = read_command(state, E1000_REG_RXDESCTAIL);
            if (state->rx_index == (int) read_command(state, E1000_REG_RXDESCHEAD)) {
                return 1;
            }
            state->rx_index = (state->rx_index + 1) % E1000_NUM_RX_DESC;
            if (state->rx[state->rx_index].status & 0x01) {
                uint8_t* pbuf = state->rx_virt[state->rx_index];
                uint16_t plen = state->rx[state->rx_index].length;

                void* packet = malloc(plen);
                memcpy(packet, pbuf, plen);

                state->rx[state->rx_index].status = 0;

                e1000_netbuf_t* buf = zalloc(sizeof(e1000_netbuf_t));
                buf->buffer = packet;
                buf->size = plen;
                enqueue_packet(state, buf);

                write_command(state, E1000_REG_RXDESCTAIL, (uint32_t) state->rx_index);
            } else {
                break;
            }
        } while (1);
    }

    return 1;
}

static int e1000_irq_handler(cpu_registers_t* r) {
    unused(r);

    list_for_each(node, e1000_list) {
        e1000_iface_t* iface = node->value;
        e1000_device_interrupt(iface);
    }
    return 0;
}

static netif_error_t send_packet(
    netif_t* iface,
    uint8_t* payload,
    size_t payload_size) {
    e1000_iface_t* net = iface->data;
    e1000_state_t* state = net->state;
    state->tx_index = read_command(state, E1000_REG_TXDESCTAIL);

    memcpy(state->tx_virt[state->tx_index], payload, payload_size);
    state->tx[state->tx_index].length = (uint16_t) payload_size;
    state->tx[state->tx_index].cmd = CMD_EOP | CMD_IFCS | CMD_RS;
    state->tx[state->tx_index].status = 0;

    state->tx_index = (state->tx_index + 1) % E1000_NUM_TX_DESC;
    write_command(state, E1000_REG_TXDESCTAIL, (uint32_t) state->tx_index);

    return IFACE_ERR_OK;
}

static void init_rx(e1000_state_t* state) {
    write_command(state, E1000_REG_RXDESCLO, state->rx_phys);
    write_command(state, E1000_REG_RXDESCHI, 0);

    write_command(state, E1000_REG_RXDESCLEN,
                  E1000_NUM_RX_DESC * sizeof(struct rx_desc));

    write_command(state, E1000_REG_RXDESCHEAD, 0);
    write_command(state, E1000_REG_RXDESCTAIL, E1000_NUM_RX_DESC - 1);

    state->rx_index = 0;

    uint32_t rctl = read_command(state, E1000_REG_RCTRL);
    rctl |= RCTL_EN;
    rctl |= RCTL_BSIZE_EXT;
    rctl &= ~(RCTL_BSIZE_16384);
    write_command(
        state,
        E1000_REG_RCTRL,
        rctl
    );
}

static void init_tx(e1000_state_t* state) {
    write_command(state, E1000_REG_TXDESCLO, state->tx_phys);
    write_command(state, E1000_REG_TXDESCHI, 0);

    write_command(state, E1000_REG_TXDESCLEN,
                  E1000_NUM_TX_DESC * sizeof(struct tx_desc));

    write_command(state, E1000_REG_TXDESCHEAD, 0);
    write_command(state, E1000_REG_TXDESCTAIL, 0);

    state->tx_index = 0;

    write_command(state, E1000_REG_TCTRL,
                  TCTL_EN | TCTL_PSP | read_command(state, E1000_REG_TCTRL));
}

static uint8_t* get_iface_mac(netif_t* iface) {
    e1000_iface_t* net = iface->data;
    e1000_state_t* state = net->state;

    return state->mac;
}

static netif_error_t iface_destroy(netif_t* iface) {
    e1000_iface_t* net = iface->data;
    ktask_cancel(net->poll_task);

    int_disable();
    list_remove(list_find(e1000_list, net));
    int_enable();

    free(net->state);
    free(net);
    free(iface->name);
    free(iface);

    return IFACE_ERR_OK;
}

static void e1000_device_init(device_entry_t* parent, pci_device_t* pci) {
    size_t idx = e1000_list->size;
    e1000_state_t* state = zalloc(sizeof(e1000_state_t));
    state->device_pci = pci->address;
    e1000_iface_t* net = zalloc(sizeof(e1000_iface_t));
    net->state = state;

    /* This seems to always be memory mapped on important devices. */
    state->mem_base = pci_read_field(state->device_pci, PCI_BAR0, 4) & 0xFFFFFFF0;

    for (size_t x = 0; x < 0x10000; x += 0x1000) {
        uintptr_t addr = (state->mem_base & 0xFFFFF000) + x;
        paging_map_dma(addr, addr);
    }

    state->rx = (void*) kpmalloc_ap(
        sizeof(struct rx_desc) * E1000_NUM_RX_DESC + 16,
        &state->rx_phys
    );

    for (int i = 0; i < E1000_NUM_RX_DESC; ++i) {
        state->rx_virt[i] = (void*) kpmalloc_ap(
            8192 + 16,
            (uintptr_t*) &state->rx[i].addr
        );
        state->rx[i].status = 0;
    }

    state->tx = (void*) kpmalloc_ap(
        sizeof(struct tx_desc) * E1000_NUM_TX_DESC + 16,
        &state->tx_phys
    );

    for (int i = 0; i < E1000_NUM_TX_DESC; ++i) {
        state->tx_virt[i] = (void*) kpmalloc_ap(
            8192 + 16,
            (uintptr_t*) &state->tx[i].addr
        );
        state->tx[i].status = 0;
        state->tx[i].cmd = (1 << 0);
    }

    uint16_t command_reg = (uint16_t) pci_read_field(state->device_pci, PCI_COMMAND, 2);
    command_reg |= (1 << 2);
    command_reg |= (1 << 0);
    pci_write_field(state->device_pci, PCI_COMMAND, 2, command_reg);

    eeprom_detect(state);
    read_mac(state);

    /* initialize */
    write_command(state, E1000_REG_CTRL, (1 << 26));

    sleep(10);

    uint32_t status = read_command(state, E1000_REG_CTRL);
    status |= (1 << 5);   /* set auto speed detection */
    status |= (1 << 6);   /* set link up */
    status &= ~(1 << 3);  /* unset link reset */
    status &= ~(1 << 31); /* unset phy reset */
    status &= ~(1 << 7);  /* unset invert loss-of-signal */
    write_command(state, E1000_REG_CTRL, status);

    /* Disables flow control */
    write_command(state, 0x0028, 0);
    write_command(state, 0x002c, 0);
    write_command(state, 0x0030, 0);
    write_command(state, 0x0170, 0);

    /* Unset flow control */
    status = read_command(state, E1000_REG_CTRL);
    status &= ~(1 << 30);
    write_command(state, E1000_REG_CTRL, status);

    sleep(10);

    state->net_queue = list_create();
    state->irq = pci_read_field(state->device_pci, PCI_INTERRUPT_LINE, 1);

    list_add(e1000_list, net);
    irq_add_handler(state->irq, &e1000_irq_handler);

    for (int i = 0; i < 128; ++i) {
        write_command(state, (uint16_t) (0x5200 + i * 4), 0);
    }

    for (int i = 0; i < 64; ++i) {
        write_command(state, (uint16_t) (0x4000 + i * 4), 0);
    }

    write_command(state, E1000_REG_RCTRL, (1 << 4));

    init_rx(state);
    init_tx(state);

    /* Twiddle interrupts */
    write_command(state, 0x00D0, 0xFF);
    write_command(state, 0x00D8, 0xFF);
    write_command(state, 0x00D0,
                  (1 << 2) | (1 << 6) | (1 << 7) | (1 << 1) | (1 << 0));

    sleep(10);

    int link_is_up = (read_command(state, E1000_REG_STATUS) & (1 << 1));

    if (!link_is_up) {
        printf(ERROR "e1000 link is not up!\n");
    }

    char* name = zalloc(16);
    sprintf(name, "intel-gig%d", (int) idx);

    netif_t* iface = netif_create(name);
    iface->class_type = IFACE_CLASS_ETHERNET;
    iface->get_mac = get_iface_mac;
    iface->send = send_packet;
    iface->destroy = iface_destroy;
    iface->handle_ioctl = e1000_ioctl;
    iface->data = net;
    net->iface = iface;

    netif_register(
        parent,
        iface
    );

    net->poll_task = ktask_repeat(1, dequeue_packet_task, net);
}

static bool is_device_e1000(uint16_t vid, uint16_t did) {
    // Check for Intel Vendor.
    if (vid != 0x8086) {
        return false;
    }

    uint16_t* checks = e1000_device_ids;
    while (*checks != 0) {
        if (*checks == did) {
            return true;
        }
        checks++;
    }

    return false;
}

void e1000_driver_setup(void) {
    e1000_list = list_create();

    list_t* list = device_query(DEVICE_CLASS_PCI);

    list_for_each(node, list) {
        device_entry_t* entry = node->value;
        pci_device_t* pci = entry->device;

        if (is_device_e1000(pci->vendor_id, pci->device_id)) {
            e1000_device_init(entry, pci);
        }
    }
}
