#include "pcnet.h"

#include <liblox/list.h>
#include <liblox/hashmap.h>
#include <liblox/printf.h>
#include <liblox/string.h>
#include <liblox/io.h>
#include <liblox/sleep.h>

#include <kernel/spin.h>
#include <kernel/cpu/task.h>

#include <kernel/arch/x86/io.h>
#include <kernel/arch/x86/heap.h>
#include <kernel/arch/x86/irq.h>

#include <kernel/arch/x86/devices/pci/pci.h>
#include <kernel/network/iface.h>

typedef struct pcnet_state {
    list_t* net_queue;
    spin_lock_t net_queue_lock;
    uint32_t device_pci;
    size_t irq;
    uint8_t mac[6];
    uint32_t io_base;
    uint32_t mem_base;
    uintptr_t buffer_phys;
    uint8_t* buffer_virt;
    uint8_t* rx_de_start;
    uint8_t* tx_de_start;
    uint8_t* tx_start;
    uint8_t* rx_start;
    uint8_t* tx_de_phys;
    uint8_t* rx_de_phys;
    uint32_t rx_phys;
    uint32_t tx_phys;
    int rx_buffer_id;
    int tx_buffer_id;
} pcnet_state_t;

#define PCNET_DE_SIZE 16
#define PCNET_BUFFER_SIZE 1548
#define PCNET_RX_COUNT 32
#define PCNET_TX_COUNT 8

typedef struct pcnet_network_iface {
    network_iface_t* iface;
    task_id poll_task;
    pcnet_state_t* state;
} pcnet_network_iface_t;

static void write_rap32(pcnet_state_t* state, uint32_t value) {
    outl((uint16_t) (state->io_base + 0x14), value);
}

static uint32_t read_csr32(pcnet_state_t* state, uint32_t csr_no) {
    write_rap32(state, csr_no);
    return inl((uint16_t) (state->io_base + 0x10));
}

static void write_csr32(pcnet_state_t* state, uint32_t csr_no, uint32_t value) {
    write_rap32(state, csr_no);
    outl((uint16_t) (state->io_base + 0x10), value);
}

static uint32_t read_bcr32(pcnet_state_t* state, uint32_t bcr_no) {
    write_rap32(state, bcr_no);
    return inl((uint16_t) (state->io_base + 0x1c));
}

static void write_bcr32(pcnet_state_t* state, uint32_t bcr_no, uint32_t value) {
    write_rap32(state, bcr_no);
    outl((uint16_t) (state->io_base + 0x1c), value);
}

static uint32_t virt_to_phys(pcnet_state_t* state, uint8_t* virt) {
    return ((uintptr_t) virt - (uintptr_t) state->buffer_virt)
           + state->buffer_phys;
}

static int driver_owns(const uint8_t* de_table, int index) {
    return (de_table[PCNET_DE_SIZE * index + 7] & 0x80) == 0;
}

static int next_tx_index(int current_tx_index) {
    int out = current_tx_index + 1;
    if (out == PCNET_TX_COUNT) {
        return 0;
    }
    return out;
}

static int next_rx_index(int current_rx_index) {
    int out = current_rx_index + 1;
    if (out == PCNET_RX_COUNT) {
        return 0;
    }
    return out;
}

static void init_descriptor(pcnet_state_t* state, int index, int is_tx) {
    uint8_t* de_table = is_tx ? state->tx_de_start : state->rx_de_start;

    memset(&de_table[index * PCNET_DE_SIZE], 0, PCNET_DE_SIZE);

    uint32_t buff_addr = is_tx ? state->tx_phys : state->rx_phys;
    *(uint32_t*) &de_table[index * PCNET_DE_SIZE] = buff_addr + index * PCNET_BUFFER_SIZE;

    uint16_t bcnt = (uint16_t) (-PCNET_BUFFER_SIZE);
    bcnt &= 0x0FFF;
    bcnt |= 0xF000;
    *(uint16_t*) &de_table[index * PCNET_DE_SIZE + 4] = bcnt;

    if (!is_tx) {
        de_table[index * PCNET_DE_SIZE + 7] = 0x80;
    }
}

static void enqueue_packet(pcnet_state_t* state, void* buffer) {
    spin_lock(state->net_queue_lock);
    list_add(state->net_queue, buffer);
    spin_unlock(state->net_queue_lock);
}

static void dequeue_packet_task(void* extra) {
    pcnet_network_iface_t* net = extra;
    pcnet_state_t* state = net->state;
    network_iface_t* iface = net->iface;

    if (state->net_queue->size == 0) {
        return;
    }

    spin_lock(state->net_queue_lock);
    list_node_t* n = list_dequeue(state->net_queue);
    void* value = n->value;
    free(n);
    spin_unlock(state->net_queue_lock);

    if (iface->handle_receive != NULL) {
        iface->handle_receive(iface, value);
    }
}

static network_iface_error_t pcnet_send_packet(network_iface_t* iface, uint8_t* payload, size_t payload_size) {
    unused(iface);

    pcnet_network_iface_t* net = iface->data;
    pcnet_state_t* state = net->state;

    if (!driver_owns(state->tx_de_start, state->tx_buffer_id)) {
        /* sleep? */
        return IFACE_ERR_FULL;
    }

    if (payload_size > PCNET_BUFFER_SIZE) {
        return IFACE_ERR_TOO_BIG;
    }

    void* ptr = (void*) (state->tx_start + state->tx_buffer_id * PCNET_BUFFER_SIZE);
    memcpy(ptr, payload, payload_size);

    state->tx_de_start[state->tx_buffer_id * PCNET_DE_SIZE + 7] |= 0x3;

    uint16_t bcnt = (uint16_t) (-payload_size);
    bcnt &= 0x0FFF;
    bcnt |= 0xF000;
    *(uint16_t*) &state->tx_de_start[state->tx_buffer_id * PCNET_DE_SIZE + 4] = bcnt;

    state->tx_de_start[state->tx_buffer_id * PCNET_DE_SIZE + 7] |= 0x80;

    write_csr32(state, 0, read_csr32(state, 0) | (1 << 3));

    state->tx_buffer_id = next_tx_index(state->tx_buffer_id);

    return IFACE_ERR_OK;
}

static network_iface_t* pcnet_iface = NULL;
static hashmap_t* pcnet_irqs = NULL;

static int pcnet_irq_handler(struct regs* r) {
    unused(r);

    uint32_t irq_id = r->int_no - 32;
    pcnet_network_iface_t* iface = hashmap_get(pcnet_irqs, (void*) irq_id);

    if (iface == NULL) {
        return 0;
    }

    pcnet_state_t* state = iface->state;
    write_csr32(state, 0, read_csr32(state, 0) | 0x0400);
    irq_ack(state->irq);

    while (driver_owns(state->rx_de_start, state->rx_buffer_id)) {
        uint16_t plen = *(uint16_t*) &state->rx_de_start[state->rx_buffer_id * PCNET_DE_SIZE + 8];

        void* pbuf = (void*) (state->rx_start + state->rx_buffer_id * PCNET_BUFFER_SIZE);

        void* packet = malloc(plen);
        memcpy(packet, pbuf, plen);
        state->rx_de_start[state->rx_buffer_id * PCNET_DE_SIZE + 7] = 0x80;

        enqueue_packet(state, packet);

        state->rx_buffer_id = next_rx_index(state->rx_buffer_id);
    }

    return 1;
}

static uint8_t* pcnet_get_iface_mac(network_iface_t* iface) {
    unused(iface);

    pcnet_network_iface_t* net = iface->data;
    pcnet_state_t* state = net->state;

    return state->mac;
}

static network_iface_error_t pcnet_iface_destroy(network_iface_t* iface) {
    pcnet_network_iface_t* net = iface->data;
    cpu_task_cancel(net->poll_task);

    free(net->state);
    free(net);
    free(iface->name);
    free(iface);

    return IFACE_ERR_OK;
}

static void pcnet_init(uint32_t device_pci) {
    pcnet_state_t* state = zalloc(sizeof(pcnet_state_t));
    state->device_pci = device_pci;

    uint16_t command_reg = (uint16_t) (
        pci_read_field(device_pci, PCI_COMMAND, 4) & 0xFFFF0000);
    if (command_reg & (1 << 2)) {
        printf(WARN "Bus mastering already enabled.\n");
    }
    command_reg |= (1 << 2);
    command_reg |= (1 << 0);
    pci_write_field(device_pci, PCI_COMMAND, 4, command_reg);

    state->io_base = pci_read_field(device_pci, PCI_BAR0, 4) & 0xFFFFFFF0;
    state->mem_base = pci_read_field(device_pci, PCI_BAR1, 4) & 0xFFFFFFF0;
    state->irq = (size_t) pci_read_field(device_pci, PCI_INTERRUPT_LINE, 1);

    /* Read MAC from EEPROM */
    state->mac[0] = inb((uint16_t) (state->io_base + 0));
    state->mac[1] = inb((uint16_t) (state->io_base + 1));
    state->mac[2] = inb((uint16_t) (state->io_base + 2));
    state->mac[3] = inb((uint16_t) (state->io_base + 3));
    state->mac[4] = inb((uint16_t) (state->io_base + 4));
    state->mac[5] = inb((uint16_t) (state->io_base + 5));

    /* Force reset */
    inl((uint16_t) (state->io_base + 0x18));
    ins((uint16_t) (state->io_base + 0x14));

    sleep(10);

    /* set 32-bit mode */
    outl((uint16_t) (state->io_base + 0x10), 0);

    /* SWSTYLE to 2 */
    uint32_t csr58 = read_csr32(state, 58);
    csr58 &= 0xFFF0;
    csr58 |= 2;
    write_csr32(state, 58, csr58);

    /* ASEL enable */
    uint32_t bcr2 = read_bcr32(state, 2);
    bcr2 |= 0x2;
    write_bcr32(state, 2, bcr2);

    state->buffer_virt = (void*) kpmalloc_ap(0x10000, &state->buffer_phys);
    if (!state->buffer_virt) {
        printf(ERROR "PCNET buffer was not allocated.\n");
        return;
    }

    state->rx_de_start = state->buffer_virt + 28;
    state->tx_de_start = state->rx_de_start + PCNET_RX_COUNT * PCNET_DE_SIZE;
    state->rx_start = state->tx_de_start + PCNET_TX_COUNT * PCNET_DE_SIZE;
    state->tx_start = state->rx_start + PCNET_RX_COUNT * PCNET_BUFFER_SIZE;

    state->rx_de_phys = (uint8_t*) virt_to_phys(state, state->rx_de_start);
    state->tx_de_phys = (uint8_t*) virt_to_phys(state, state->tx_de_start);
    state->rx_phys = virt_to_phys(state, state->rx_start);
    state->tx_phys = virt_to_phys(state, state->tx_start);

    /* set up descriptors */
    for (int i = 0; i < PCNET_RX_COUNT; i++) {
        init_descriptor(state, i, 0);
    }

    for (int i = 0; i < PCNET_TX_COUNT; i++) {
        init_descriptor(state, i, 1);
    }

    /* Set up device configuration structure */
    ((uint16_t*) &state->buffer_virt[0])[0] = 0x0000;
    state->buffer_virt[2] = 5 << 4; /* RLEN << 4 */
    state->buffer_virt[3] = 3 << 4; /* TLEN << 4 */
    state->buffer_virt[4] = state->mac[0];
    state->buffer_virt[5] = state->mac[1];
    state->buffer_virt[6] = state->mac[2];
    state->buffer_virt[7] = state->mac[3];
    state->buffer_virt[8] = state->mac[4];
    state->buffer_virt[9] = state->mac[5];

    state->buffer_virt[10] = 0; /* reserved */
    state->buffer_virt[11] = 0; /* reserved */

    state->buffer_virt[12] = 0;
    state->buffer_virt[13] = 0;
    state->buffer_virt[14] = 0;
    state->buffer_virt[15] = 0;
    state->buffer_virt[16] = 0;
    state->buffer_virt[17] = 0;
    state->buffer_virt[18] = 0;
    state->buffer_virt[19] = 0;

    ((uint32_t*) &state->buffer_virt[20])[0] = (uint32_t) state->rx_de_phys;
    ((uint32_t*) &state->buffer_virt[24])[0] = (uint32_t) state->tx_de_phys;

    /* Configure network */
    state->net_queue = list_create();

    write_csr32(state, 1, 0xFFFF & state->buffer_phys);
    write_csr32(state, 2, 0xFFFF & (state->buffer_phys >> 16));

    uint16_t csr3 = (uint16_t) read_csr32(state, 3);
    if (csr3 & (1 << 10)) {
        csr3 ^= (1 << 10);
    }
    if (csr3 & (1 << 2)) {
        csr3 ^= (1 << 2);
    }
    csr3 |= (1 << 9);
    csr3 |= (1 << 8);
    write_csr32(state, 3, csr3); /* Disable interrupt on init */
    write_csr32(state, 4, read_csr32(state, 4) | (1 << 1) | (1 << 12) | (1 << 14)); /* pad */

    write_csr32(state, 0, read_csr32(state, 0) | (1 << 0) | (1 << 6)); /* do it */

    uint64_t start_time;
    asm volatile (".byte 0x0f, 0x31" : "=A" (start_time));

    uint32_t status;
    while (((status = read_csr32(state, 0)) & (1 << 8)) == 0) {
        uint64_t now_time;
        asm volatile (".byte 0x0f, 0x31" : "=A" (now_time));
        if (now_time - start_time > 0x10000) {
            printf(ERROR "Could not initialize PCNet card, status is 0x%4x\n", status);
            return;
        }
    }

    /* Start card */
    uint16_t csr0 = (uint16_t) read_csr32(state, 0);
    if (csr0 & (1 << 0)) {
        csr0 ^= (1 << 0);
    }

    if (csr0 & (1 << 2)) {
        csr0 ^= (1 << 2);
    }
    csr0 |= (1 << 1);
    write_csr32(state, 0, csr0);

    size_t idx = hashmap_count(pcnet_irqs);
    char* name = zalloc(16);
    sprintf(name, "pcnet%d", idx);
    pcnet_iface = network_iface_create(name);

    pcnet_network_iface_t* dat = zalloc(sizeof(pcnet_network_iface_t));
    dat->state = state;
    dat->iface = pcnet_iface;

    hashmap_set(pcnet_irqs, (void*) state->irq, dat);
    irq_add_handler(state->irq, &pcnet_irq_handler);

    pcnet_iface->class_type = IFACE_CLASS_ETHERNET;
    pcnet_iface->get_mac = pcnet_get_iface_mac;
    pcnet_iface->send = pcnet_send_packet;
    pcnet_iface->destroy = pcnet_iface_destroy;
    pcnet_iface->data = dat;

    network_iface_register(pcnet_iface);

    dat->poll_task = cpu_task_repeat(1, dequeue_packet_task, dat);
}

static void find_pcnet(uint32_t device, uint16_t vendor_id, uint16_t device_id, void* extra) {
    unused(extra);

    if ((vendor_id == 0x1022) && (device_id == 0x2000)) {
        pcnet_init(device);
    }
}

void pcnet_setup(void) {
    pcnet_irqs = hashmap_create_int(5);
    pci_scan(&find_pcnet, -1, NULL);
}
