#include <liblox/list.h>
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

static list_t* net_queue = NULL;
static spin_lock_t net_queue_lock = {0};

static uint32_t pcnet_device_pci = 0x00000000;
static uint32_t pcnet_io_base = 0;
static uint32_t pcnet_mem_base = 0;
static int pcnet_irq;
static uint8_t mac[6];

static uintptr_t pcnet_buffer_phys;
static uint8_t* pcnet_buffer_virt;

static uint8_t* pcnet_rx_de_start;
static uint8_t* pcnet_tx_de_start;
static uint8_t* pcnet_rx_start;
static uint8_t* pcnet_tx_start;

static uint32_t pcnet_rx_de_phys;
static uint32_t pcnet_tx_de_phys;
static uint32_t pcnet_rx_phys;
static uint32_t pcnet_tx_phys;

static int pcnet_rx_buffer_id = 0;
static int pcnet_tx_buffer_id = 0;

#define PCNET_DE_SIZE 16
#define PCNET_BUFFER_SIZE 1548
#define PCNET_RX_COUNT 32
#define PCNET_TX_COUNT 8

typedef struct pcnet_network_iface {
    network_iface_t* iface;
    task_id poll_task;
} pcnet_network_iface_t;

static void find_pcnet(uint32_t device, uint16_t vendorid, uint16_t deviceid, void* extra) {
    if ((vendorid == 0x1022) && (deviceid == 0x2000)) {
        *((uint32_t*) extra) = device;
    }
}

static void write_rap32(uint32_t value) {
    outl((uint16_t) (pcnet_io_base + 0x14), value);
}

static uint32_t read_csr32(uint32_t csr_no) {
    write_rap32(csr_no);
    return inl((uint16_t) (pcnet_io_base + 0x10));
}

static void write_csr32(uint32_t csr_no, uint32_t value) {
    write_rap32(csr_no);
    outl((uint16_t) (pcnet_io_base + 0x10), value);
}

static uint32_t read_bcr32(uint32_t bcr_no) {
    write_rap32(bcr_no);
    return inl((uint16_t) (pcnet_io_base + 0x1c));
}

static void write_bcr32(uint32_t bcr_no, uint32_t value) {
    write_rap32(bcr_no);
    outl((uint16_t) (pcnet_io_base + 0x1c), value);
}

static uint32_t virt_to_phys(uint8_t* virt) {
    return ((uintptr_t) virt - (uintptr_t) pcnet_buffer_virt) + pcnet_buffer_phys;
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

static void init_descriptor(int index, int is_tx) {
    uint8_t* de_table = is_tx ? pcnet_tx_de_start : pcnet_rx_de_start;

    memset(&de_table[index * PCNET_DE_SIZE], 0, PCNET_DE_SIZE);

    uint32_t buf_addr = is_tx ? pcnet_tx_phys : pcnet_rx_phys;
    *(uint32_t*) &de_table[index * PCNET_DE_SIZE] = buf_addr + index * PCNET_BUFFER_SIZE;

    uint16_t bcnt = (uint16_t) (-PCNET_BUFFER_SIZE);
    bcnt &= 0x0FFF;
    bcnt |= 0xF000;
    *(uint16_t*) &de_table[index * PCNET_DE_SIZE + 4] = bcnt;

    if (!is_tx) {
        de_table[index * PCNET_DE_SIZE + 7] = 0x80;
    }
}

static void enqueue_packet(void* buffer) {
    spin_lock(net_queue_lock);
    list_add(net_queue, buffer);
    spin_unlock(net_queue_lock);
}

static void dequeue_packet_task(void* extra) {
    pcnet_network_iface_t* net = extra;
    network_iface_t* iface = net->iface;

    if (net_queue->size == 0) {
        return;
    }

    spin_lock(net_queue_lock);
    list_node_t* n = list_dequeue(net_queue);
    void* value = n->value;
    free(n);
    spin_unlock(net_queue_lock);

    if (iface->handle_receive != NULL) {
        iface->handle_receive(iface, value);
    }
}

uint8_t* pcnet_get_mac() {
    return mac;
}

static network_iface_error_t pcnet_send_packet(network_iface_t* iface, uint8_t* payload, size_t payload_size) {
    unused(iface);

    if (!driver_owns(pcnet_tx_de_start, pcnet_tx_buffer_id)) {
        /* sleep? */
        return IFACE_ERR_FULL;
    }

    if (payload_size > PCNET_BUFFER_SIZE) {
        return IFACE_ERR_TOO_BIG;
    }

    void* ptr = (void*) (pcnet_tx_start + pcnet_tx_buffer_id * PCNET_BUFFER_SIZE);
    memcpy(ptr, payload, payload_size);

    pcnet_tx_de_start[pcnet_tx_buffer_id * PCNET_DE_SIZE + 7] |= 0x3;

    uint16_t bcnt = (uint16_t) (-payload_size);
    bcnt &= 0x0FFF;
    bcnt |= 0xF000;
    *(uint16_t*) &pcnet_tx_de_start[pcnet_tx_buffer_id * PCNET_DE_SIZE + 4] = bcnt;

    pcnet_tx_de_start[pcnet_tx_buffer_id * PCNET_DE_SIZE + 7] |= 0x80;

    write_csr32(0, read_csr32(0) | (1 << 3));

    pcnet_tx_buffer_id = next_tx_index(pcnet_tx_buffer_id);

    return IFACE_ERR_OK;
}

static network_iface_t* pcnet_iface = NULL;

static int pcnet_irq_handler(struct regs* r) {
    unused(r);

    write_csr32(0, read_csr32(0) | 0x0400);

    while (driver_owns(pcnet_rx_de_start, pcnet_rx_buffer_id)) {
        uint16_t plen = *(uint16_t*) &pcnet_rx_de_start[pcnet_rx_buffer_id * PCNET_DE_SIZE + 8];

        void* pbuf = (void*) (pcnet_rx_start + pcnet_rx_buffer_id * PCNET_BUFFER_SIZE);

        void* packet = malloc(plen);
        memcpy(packet, pbuf, plen);
        pcnet_rx_de_start[pcnet_rx_buffer_id * PCNET_DE_SIZE + 7] = 0x80;

        enqueue_packet(packet);

        pcnet_rx_buffer_id = next_rx_index(pcnet_rx_buffer_id);
    }

    return 1;
}

static uint8_t* pcnet_get_iface_mac(network_iface_t* iface) {
    unused(iface);

    return mac;
}

static network_iface_error_t pcnet_iface_destroy(network_iface_t* iface) {
    pcnet_network_iface_t* net = iface->data;
    cpu_task_cancel(net->poll_task);
    return IFACE_ERR_OK;
}

static void pcnet_init(void* data) {
    unused(data);

    uint16_t command_reg = (uint16_t) (
        pci_read_field(pcnet_device_pci, PCI_COMMAND, 4) & 0xFFFF0000);
    if (command_reg & (1 << 2)) {
        printf(INFO "Bus mastering already enabled.\n");
    }
    command_reg |= (1 << 2);
    command_reg |= (1 << 0);
    pci_write_field(pcnet_device_pci, PCI_COMMAND, 4, command_reg);

    pcnet_io_base = pci_read_field(pcnet_device_pci, PCI_BAR0, 4) & 0xFFFFFFF0;
    pcnet_mem_base = pci_read_field(pcnet_device_pci, PCI_BAR1, 4) & 0xFFFFFFF0;

    pcnet_irq = pci_read_field(pcnet_device_pci, PCI_INTERRUPT_LINE, 1);
    irq_add_handler((size_t) pcnet_irq, &pcnet_irq_handler);

    /* Read MAC from EEPROM */
    mac[0] = inb((uint16_t) (pcnet_io_base + 0));
    mac[1] = inb((uint16_t) (pcnet_io_base + 1));
    mac[2] = inb((uint16_t) (pcnet_io_base + 2));
    mac[3] = inb((uint16_t) (pcnet_io_base + 3));
    mac[4] = inb((uint16_t) (pcnet_io_base + 4));
    mac[5] = inb((uint16_t) (pcnet_io_base + 5));

    /* Force reset */
    inl((uint16_t) (pcnet_io_base + 0x18));
    ins((uint16_t) (pcnet_io_base + 0x14));

    sleep(10);

    /* set 32-bit mode */
    outl((uint16_t) (pcnet_io_base + 0x10), 0);

    /* SWSTYLE to 2 */
    uint32_t csr58 = read_csr32(58);
    csr58 &= 0xFFF0;
    csr58 |= 2;
    write_csr32(58, csr58);

    /* ASEL enable */
    uint32_t bcr2 = read_bcr32(2);
    bcr2 |= 0x2;
    write_bcr32(2, bcr2);

    if (!pcnet_buffer_virt) {
        return;
    }

    pcnet_rx_de_start = pcnet_buffer_virt + 28;
    pcnet_tx_de_start = pcnet_rx_de_start + PCNET_RX_COUNT * PCNET_DE_SIZE;
    pcnet_rx_start = pcnet_tx_de_start + PCNET_TX_COUNT * PCNET_DE_SIZE;
    pcnet_tx_start = pcnet_rx_start + PCNET_RX_COUNT * PCNET_BUFFER_SIZE;

    pcnet_rx_de_phys = virt_to_phys(pcnet_rx_de_start);
    pcnet_tx_de_phys = virt_to_phys(pcnet_tx_de_start);
    pcnet_rx_phys = virt_to_phys(pcnet_rx_start);
    pcnet_tx_phys = virt_to_phys(pcnet_tx_start);

    /* set up descriptors */
    for (int i = 0; i < PCNET_RX_COUNT; i++) {
        init_descriptor(i, 0);
    }

    for (int i = 0; i < PCNET_TX_COUNT; i++) {
        init_descriptor(i, 1);
    }

    /* Set up device configuration structure */
    ((uint16_t*) &pcnet_buffer_virt[0])[0] = 0x0000;
    pcnet_buffer_virt[2] = 5 << 4; /* RLEN << 4 */
    pcnet_buffer_virt[3] = 3 << 4; /* TLEN << 4 */
    pcnet_buffer_virt[4] = mac[0];
    pcnet_buffer_virt[5] = mac[1];
    pcnet_buffer_virt[6] = mac[2];
    pcnet_buffer_virt[7] = mac[3];
    pcnet_buffer_virt[8] = mac[4];
    pcnet_buffer_virt[9] = mac[5];

    pcnet_buffer_virt[10] = 0; /* reserved */
    pcnet_buffer_virt[11] = 0; /* reserved */

    pcnet_buffer_virt[12] = 0;
    pcnet_buffer_virt[13] = 0;
    pcnet_buffer_virt[14] = 0;
    pcnet_buffer_virt[15] = 0;
    pcnet_buffer_virt[16] = 0;
    pcnet_buffer_virt[17] = 0;
    pcnet_buffer_virt[18] = 0;
    pcnet_buffer_virt[19] = 0;

    ((uint32_t*) &pcnet_buffer_virt[20])[0] = pcnet_rx_de_phys;
    ((uint32_t*) &pcnet_buffer_virt[24])[0] = pcnet_tx_de_phys;

    /* Configure network */
    net_queue = list_create();

    write_csr32(1, 0xFFFF & pcnet_buffer_phys);
    write_csr32(2, 0xFFFF & (pcnet_buffer_phys >> 16));

    uint16_t csr3 = (uint16_t) read_csr32(3);
    if (csr3 & (1 << 10)) {
        csr3 ^= (1 << 10);
    }
    if (csr3 & (1 << 2)) {
        csr3 ^= (1 << 2);
    }
    csr3 |= (1 << 9);
    csr3 |= (1 << 8);
    write_csr32(3, csr3); /* Disable interrupt on init */
    write_csr32(4, read_csr32(4) | (1 << 1) | (1 << 12) | (1 << 14)); /* pad */

    write_csr32(0, read_csr32(0) | (1 << 0) | (1 << 6)); /* do it */

    uint64_t start_time;
    asm volatile (".byte 0x0f, 0x31" : "=A" (start_time));

    uint32_t status;
    while (((status = read_csr32(0)) & (1 << 8)) == 0) {
        uint64_t now_time;
        asm volatile (".byte 0x0f, 0x31" : "=A" (now_time));
        if (now_time - start_time > 0x10000) {
            printf(ERROR "Could not initialize PCNet card, status is 0x%4x\n", status);
            return;
        }
    }

    /* Start card */
    uint16_t csr0 = (uint16_t) read_csr32(0);
    if (csr0 & (1 << 0)) {
        csr0 ^= (1 << 0);
    }

    if (csr0 & (1 << 2)) {
        csr0 ^= (1 << 2);
    }
    csr0 |= (1 << 1);
    write_csr32(0, csr0);
    pcnet_get_mac();

    pcnet_iface = network_iface_create("pcnet");

    pcnet_network_iface_t* dat = zalloc(sizeof(pcnet_network_iface_t));
    dat->iface = pcnet_iface;
    pcnet_iface->class_type = IFACE_CLASS_ETHERNET;
    pcnet_iface->get_mac = pcnet_get_iface_mac;
    pcnet_iface->send = pcnet_send_packet;
    pcnet_iface->destroy = pcnet_iface_destroy;
    pcnet_iface->data = dat;

    network_iface_register(pcnet_iface);

    dat->poll_task = cpu_task_repeat(1, dequeue_packet_task, dat);
}

void pcnet_setup(void) {
    pci_scan(&find_pcnet, -1, &pcnet_device_pci);

    if (!pcnet_device_pci) {
        return;
    }

    /* This fits 32x1548 (rx) + 8x1548 (tx) + 32x16 (rx DE) + 8x16 (tx DE) */
    pcnet_buffer_virt = (void*) kpmalloc_ap(0x10000, &pcnet_buffer_phys);

    pcnet_init(NULL);
}
