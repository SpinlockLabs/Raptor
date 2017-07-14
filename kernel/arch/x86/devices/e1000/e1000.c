#include "e1000.h"

#include <liblox/common.h>
#include <liblox/list.h>
#include <liblox/hashmap.h>
#include <liblox/string.h>
#include <liblox/sleep.h>
#include <liblox/printf.h>

#include <kernel/spin.h>
#include <kernel/cpu/task.h>
#include <kernel/network/iface.h>

#include <kernel/arch/x86/irq.h>
#include <kernel/arch/x86/paging.h>
#include <kernel/arch/x86/heap.h>
#include <kernel/arch/x86/devices/pci/pci.h>

static uint32_t mmio_read32(uintptr_t addr) {
    return *((volatile uint32_t *)(addr));
}

static void mmio_write32(uintptr_t addr, uint32_t val) {
    (*((volatile uint32_t *)(addr))) = val;
}

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

struct rx_desc {
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint16_t checksum;
    volatile uint8_t status;
    volatile uint8_t errors;
    volatile uint16_t special;
} packed; /* this looks like it should pack fine as-is */

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
    network_iface_t* iface;
    e1000_state_t* state;
    task_id poll_task;
} e1000_iface_t;

static void write_command(e1000_state_t* state, uint16_t addr, uint32_t val) {
    mmio_write32(state->mem_base + addr, val);
}

static uint32_t read_command(e1000_state_t* state, uint16_t addr) {
    return mmio_read32(state->mem_base + addr);
}

static void enqueue_packet(e1000_state_t* state, void *buffer) {
    spin_lock(state->net_queue_lock);
    list_add(state->net_queue, buffer);
    spin_unlock(state->net_queue_lock);
}

static void dequeue_packet_task(void* data) {
    e1000_iface_t* net = data;
    e1000_state_t* state = net->state;

    if (state->net_queue->size == 0) {
        return;
    }

    spin_lock(state->net_queue_lock);
    list_node_t* n = list_dequeue(state->net_queue);
    void* value = n->value;
    free(n);
    spin_unlock(state->net_queue_lock);

    network_iface_t* iface = net->iface;
    if (iface->handle_receive != NULL) {
        iface->handle_receive(iface, (uint8_t*) value);
    }
}

#define E1000_REG_CTRL 0x0000
#define E1000_REG_STATUS 0x0008
#define E1000_REG_EEPROM 0x0014
#define E1000_REG_CTRL_EXT 0x0018

#define E1000_REG_RCTRL 0x0100
#define E1000_REG_RXDESCLO 0x2800
#define E1000_REG_RXDESCHI 0x2804
#define E1000_REG_RXDESCLEN 0x2808
#define E1000_REG_RXDESCHEAD 0x2810
#define E1000_REG_RXDESCTAIL 0x2818

#define E1000_REG_TCTRL 0x0400
#define E1000_REG_TXDESCLO 0x3800
#define E1000_REG_TXDESCHI 0x3804
#define E1000_REG_TXDESCLEN 0x3808
#define E1000_REG_TXDESCHEAD 0x3810
#define E1000_REG_TXDESCTAIL 0x3818

#define RCTL_EN (1 << 1)            /* Receiver Enable */
#define RCTL_SBP (1 << 2)           /* Store Bad Packets */
#define RCTL_UPE (1 << 3)           /* Unicast Promiscuous Enabled */
#define RCTL_MPE (1 << 4)           /* Multicast Promiscuous Enabled */
#define RCTL_LPE (1 << 5)           /* Long Packet Reception Enable */
#define RCTL_LBM_NONE (0 << 6)      /* No Loopback */
#define RCTL_LBM_PHY (3 << 6)       /* PHY or external SerDesc loopback */
#define RTCL_RDMTS_HALF (0 << 8)    /* Free Buffer Threshold is 1/2 of RDLEN */
#define RTCL_RDMTS_QUARTER (1 << 8) /* Free Buffer Threshold is 1/4 of RDLEN   \
                                       */
#define RTCL_RDMTS_EIGHTH (2 << 8)  /* Free Buffer Threshold is 1/8 of RDLEN */
#define RCTL_MO_36 (0 << 12)        /* Multicast Offset - bits 47:36 */
#define RCTL_MO_35 (1 << 12)        /* Multicast Offset - bits 46:35 */
#define RCTL_MO_34 (2 << 12)        /* Multicast Offset - bits 45:34 */
#define RCTL_MO_32 (3 << 12)        /* Multicast Offset - bits 43:32 */
#define RCTL_BAM (1 << 15)          /* Broadcast Accept Mode */
#define RCTL_VFE (1 << 18)          /* VLAN Filter Enable */
#define RCTL_CFIEN (1 << 19)        /* Canonical Form Indicator Enable */
#define RCTL_CFI (1 << 20)          /* Canonical Form Indicator Bit Value */
#define RCTL_DPF (1 << 22)          /* Discard Pause Frames */
#define RCTL_PMCF (1 << 23)         /* Pass MAC Control Frames */
#define RCTL_SECRC (1 << 26)        /* Strip Ethernet CRC */

#define RCTL_BSIZE_256 (3 << 16)
#define RCTL_BSIZE_512 (2 << 16)
#define RCTL_BSIZE_1024 (1 << 16)
#define RCTL_BSIZE_2048 (0 << 16)
#define RCTL_BSIZE_4096 ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192 ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384 ((1 << 16) | (1 << 25))

#define TCTL_EN (1 << 1)      /* Transmit Enable */
#define TCTL_PSP (1 << 3)     /* Pad Short Packets */
#define TCTL_CT_SHIFT 4       /* Collision Threshold */
#define TCTL_COLD_SHIFT 12    /* Collision Distance */
#define TCTL_SWXOFF (1 << 22) /* Software XOFF Transmission */
#define TCTL_RTLC (1 << 24)   /* Re-transmit on Late Collision */

#define CMD_EOP (1 << 0)  /* End of Packet */
#define CMD_IFCS (1 << 1) /* Insert FCS */
#define CMD_IC (1 << 2)   /* Insert Checksum */
#define CMD_RS (1 << 3)   /* Report Status */
#define CMD_RPS (1 << 4)  /* Report Packet Sent */
#define CMD_VLE (1 << 6)  /* VLAN Packet Enable */
#define CMD_IDE (1 << 7)  /* Interrupt Delay Enable */

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
    write_command(state, E1000_REG_EEPROM, 1 | ((uint32_t)(addr) << 8));
    while (!((temp = read_command(state, E1000_REG_EEPROM)) & (1 << 4))) {}
    return (uint16_t) ((temp >> 16) & 0xFFFF);
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
        uint8_t *mac_addr = (uint8_t*) (state->mem_base + 0x5400);
        for (int i = 0; i < 6; ++i) {
            state->mac[i] = mac_addr[i];
        }
    }
}

static hashmap_t* e1000_irqs = NULL;

static int e1000_irq_handler(cpu_registers_t* r) {
    uint32_t irq_id = r->int_no - 32;
    e1000_iface_t* iface = hashmap_get(e1000_irqs, (void*) irq_id);

    if (iface == NULL) {
        return 0;
    }

    e1000_state_t* state = iface->state;
    uint32_t status = read_command(state, 0xc0);

    irq_ack(irq_id);

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
                uint8_t *pbuf = state->rx_virt[state->rx_index];
                uint16_t plen = state->rx[state->rx_index].length;

                void *packet = malloc(plen);
                memcpy(packet, pbuf, plen);

                state->rx[state->rx_index].status = 0;

                enqueue_packet(state, packet);

                write_command(state, E1000_REG_RXDESCTAIL, (uint32_t) state->rx_index);
            } else {
                break;
            }
        } while (1);
    }

    return 1;
}

static network_iface_error_t send_packet(network_iface_t* iface, uint8_t* payload, size_t payload_size) {
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

    write_command(
        state,
        E1000_REG_RCTRL,
        RCTL_EN | (read_command(state, E1000_REG_RCTRL) & (
            ~((1 << 17) | (1 << 16))))
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

static uint8_t* get_iface_mac(network_iface_t* iface) {
    e1000_iface_t* net = iface->data;
    e1000_state_t* state = net->state;

    return state->mac;
}

static network_iface_error_t iface_destroy(network_iface_t* iface) {
    e1000_iface_t* net = iface->data;
    cpu_task_cancel(net->poll_task);

    hashmap_remove(e1000_irqs, (void*) net->state->irq);
    free(net->state);
    free(net);
    free(iface->name);
    free(iface);

    return IFACE_ERR_OK;
}

static void e1000_device_init(uint32_t device_pci) {
    size_t idx = hashmap_count(e1000_irqs);
    e1000_state_t* state = zalloc(sizeof(e1000_state_t));
    state->device_pci = device_pci;
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
        &state->rx_phys);

    for (int i = 0; i < E1000_NUM_RX_DESC; ++i) {
        state->rx_virt[i] = (void*) kpmalloc_ap(
            8192 + 16, (uint32_t*) &state->rx[i].addr);
        state->rx[i].status = 0;
    }

    state->tx = (void*) kpmalloc_ap(
        sizeof(struct tx_desc) * E1000_NUM_TX_DESC + 16,
        &state->tx_phys);

    for (int i = 0; i < E1000_NUM_TX_DESC; ++i) {
        state->tx_virt[i] = (void*) kpmalloc_ap(
            8192 + 16,
            (uint32_t*) &state->tx[i].addr
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

    hashmap_set(e1000_irqs, (void*) state->irq, net);
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

    network_iface_t* iface = network_iface_create(name);
    iface->class_type = IFACE_CLASS_ETHERNET;
    iface->get_mac = get_iface_mac;
    iface->send = send_packet;
    iface->destroy = iface_destroy;
    iface->data = net;
    net->iface = iface;

    network_iface_register(iface);

    net->poll_task = cpu_task_repeat(1, dequeue_packet_task, net);
}

static void find_e1000(uint32_t device, uint16_t vid, uint16_t did,
                       void *extra) {
    unused(extra);

    if ((vid == 0x8086) &&
        (did == 0x100e || did == 0x1004 || did == 0x100f)) {
        e1000_device_init(device);
    }
}

void e1000_setup(void) {
    e1000_irqs = hashmap_create_int(5);
    pci_scan(&find_e1000, -1, NULL);
}
