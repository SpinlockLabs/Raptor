#include "debug.h"
#include "heap.h"
#include "paging.h"

#include <liblox/net.h>

#include <kernel/tty.h>

#include <kernel/network/iface.h>
#include <kernel/network/ip.h>
#include <kernel/network/udp.h>
#include <kernel/network/ethernet.h>

#include <kernel/dispatch/events.h>

#include <kernel/debug/console.h>

#include <kernel/arch/x86/devices/pci/pci.h>

#include <kernel/rkmalloc/rkmalloc.h>

static void debug_kpused(tty_t* tty, const char* input) {
    unused(input);

    size_t size = kpused();
    int kb = size / 1024;

    tty_printf(tty, "Used: %d bytes, %d kb\n", size, kb);
}

static void debug_pcnet_fake_packet(tty_t* tty, const char* input) {
    size_t len = strlen(input);

    if (len == 0) {
        input = "Hello World";
        len = strlen(input);
    }

    uint8_t mac[6];
    network_iface_t* iface = network_iface_get("pcnet0");
    network_iface_get_mac(iface, mac);

    struct ethernet_packet eth = {
        .source = {mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]},
        .destination = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        .type = htons(0x0800)
    };

    uint8_t anon[] = {0, 0, 0, 0};
    uint8_t broadcast[] = {255, 255, 255, 50};

    uint16_t ip_len = htons(sizeof(struct ipv4_packet) + sizeof(struct udp_packet) + len);

    struct ipv4_packet ip = {
        .version_ihl = ((0x4 << 4) | (0x5 << 0)),
        .dscp_ecn = 0,
        .length = ip_len,
        .ident = htons(1),
        .ttl = 0x40,
        .protocol = IPV4_PROTOCOL_UDP,
        .checksum = 0,
        .source = ((uint32_t*) anon)[0],
        .destination = ((uint32_t*) broadcast)[0]
    };

    ip.checksum = htons(ipv4_calculate_checksum(&ip));

    uint16_t udp_len = htons(sizeof(struct udp_packet) + len);

    struct udp_packet udp = {
        .source_port = htons(5902),
        .destination_port = htons(10),
        .length = udp_len,
        .checksum = 0
    };

    size_t complete_size = sizeof(ethernet_packet_t)
                           + sizeof(struct ipv4_packet)
                           + sizeof(struct udp_packet)
                           + len;

    uint8_t* buff = zalloc(complete_size);
    uint32_t offset = 0;

    memcpy(&buff[offset], &eth, sizeof(ethernet_packet_t));
    offset += sizeof(ethernet_packet_t);
    memcpy(&buff[offset], &ip, sizeof(struct ipv4_packet));
    offset += sizeof(struct ipv4_packet);
    memcpy(&buff[offset], &udp, sizeof(struct udp_packet));
    offset += sizeof(struct udp_packet);
    memcpy(&buff[offset], input, len);

    network_iface_error_t error = network_iface_send(iface, buff, offset);
    if (error != IFACE_ERR_OK) {
        tty_printf(tty, "Failed to send packet: %d\n", error);
    } else {
        tty_printf(tty, "Sent %d total bytes.\n", complete_size);
    }
    free(buff);
}

static void debug_page_stats(tty_t* tty, const char* input) {
    unused(input);

    uintptr_t memory_total = paging_memory_total();
    uintptr_t memory_used = paging_memory_used();

    tty_printf(tty,
               "Total Memory: %d bytes, %d kb\n",
               memory_total,
               (uintptr_t) (memory_total / 1024)
    );

    tty_printf(tty,
               "Used Memory: %d bytes, %d kb\n",
               memory_used,
               (uintptr_t) (memory_used / 1024)
    );
}

static void debug_pcnet_mac(tty_t* tty, const char* input) {
    unused(input);

    network_iface_t* iface = network_iface_get("pcnet0");

    if (iface == NULL) {
        tty_printf(tty, "PCNET is not configured.\n");
        return;
    }

    uint8_t mac[6] = {0};
    if (network_iface_get_mac(iface, mac) != IFACE_ERR_OK) {
        tty_printf(tty, "Network interface error.\n");
        return;
    }

    tty_printf(tty,
               "%2x:%2x:%2x:%2x:%2x:%2x\n",
               mac[0],
               mac[1],
               mac[2],
               mac[3],
               mac[4],
               mac[5]
    );
}

static void pci_show_simple(uint32_t loc, uint16_t vid, uint16_t did, void* extra) {
    tty_t* tty = extra;

    const char* vendor = pci_vendor_lookup(vid);
    const char* dev = pci_device_lookup(vid, did);

    if (vendor == NULL) {
        vendor = "Unknown";
    }

    if (dev == NULL) {
        dev = "Unknown";
    }

    tty_printf(tty,
               "(0x%x) [%s] (0x%x) by [%s] (0x%x)\n",
               loc, dev, did, vendor, vid);
}

static void debug_pci_list(tty_t* tty, const char* input) {
    unused(input);

    pci_scan(pci_show_simple, -1, tty);
}

static void debug_fake_event(tty_t* tty, const char* input) {
    unused(tty);

    event_dispatch((char*) input, NULL);
}

void debug_x86_init(void) {
    debug_console_register_command("kpused", debug_kpused);
    debug_console_register_command("pci-list", debug_pci_list);
    debug_console_register_command("page-stats", debug_page_stats);
    debug_console_register_command("pcnet-mac", debug_pcnet_mac);
    debug_console_register_command("fake-event", debug_fake_event);
    debug_console_register_command("pcnet-packet", debug_pcnet_fake_packet);
}
