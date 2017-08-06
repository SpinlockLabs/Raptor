#include "arp.h"
#include "config.h"
#include "stack.h"
#include "log.h"

#include <liblox/hashmap.h>
#include <liblox/net.h>

#include <kernel/dispatch/events.h>
#include <kernel/network/ethernet.h>

#define copy_mac(target, src) \
        for (uint i = 0; i <= 5; i++) { \
            (target)[i] = (src)[i]; \
        }

typedef struct arp_state {
    hashmap_t* table;
} arp_state_t;

static arp_state_t* get_state(network_iface_t* iface) {
    if (iface == NULL) {
        return NULL;
    }
    return hashmap_get(iface->manager_data, "arp");
}

typedef struct arp_entry {
    ipv4_address_t address;
    uint8_t mac[6];
} arp_entry_t;

static void ask(network_iface_t* iface, uint32_t addr) {
    if (iface->class_type != IFACE_CLASS_ETHERNET) {
        return;
    }

    netconf_t* conf = netconf_get(iface);
    netconf_lock(conf);
    uint32_t source = conf->ipv4.source;
    netconf_unlock(conf);

    if (source == 0) {
        return;
    }

    addr = ntohl(addr);
    source = ntohl(source);

    arp_packet_t* pkt = zalloc(sizeof(arp_packet_t));

    pkt->htype = ntohs(1);
    pkt->proto = ntohs(0x0800);
    pkt->hlen = 6;
    pkt->plen = 4;
    pkt->oper = ntohs(1);

    uint8_t mac[6] = {0};
    network_iface_error_t error = network_iface_get_mac(iface, mac);

    if (error != IFACE_ERR_OK) {
        return;
    }

    copy_mac(pkt->sender_ha, mac);

    pkt->sender_ip = source;
    pkt->target_ip = addr;

    dbg(
        "Asking interface %s who owns %d.%d.%d.%d\n",
        iface->name,
        ip_cp(addr)
    );

    network_stack_send_packet(
        iface,
        (uint8_t*) pkt,
        sizeof(arp_packet_t),
        PACKET_CLASS_ARP,
        0
    );
}

static void tell(network_iface_t* iface, uint32_t who, const uint8_t who_hw[6]) {
    if (iface->class_type != IFACE_CLASS_ETHERNET) {
        return;
    }

    netconf_t* conf = netconf_get(iface);
    netconf_lock(conf);
    uint32_t source = conf->ipv4.source;
    netconf_unlock(conf);

    if (source == 0) {
        return;
    }

    source = ntohl(source);
    who = ntohl(who);

    arp_packet_t* pkt = zalloc(sizeof(arp_packet_t));

    pkt->htype = ntohs(1);
    pkt->proto = ntohs(0x0800);
    pkt->hlen = 6;
    pkt->plen = 4;
    pkt->oper = ntohs(2);

    uint8_t mac[6] = {0};
    network_iface_error_t error = network_iface_get_mac(iface, mac);

    if (error != IFACE_ERR_OK) {
        return;
    }

    copy_mac(pkt->sender_ha, mac);
    copy_mac(pkt->target_ha, who_hw);

    pkt->sender_ip = source;
    pkt->target_ip = who;

    network_stack_send_packet(
        iface,
        (uint8_t*) pkt,
        sizeof(arp_packet_t),
        PACKET_CLASS_ARP,
        0
    );
}

static void handle_config_change(void* event, void* extra) {
    unused(extra);

    network_iface_t* iface = event;
    netconf_t* conf = netconf_get(iface);
    netconf_lock(conf);
    uint32_t gw = conf->ipv4.gateway;
    netconf_unlock(conf);

    if (gw == 0) {
        return;
    }

    ask(iface, gw);
}

static void handle_potential_arp(void* event, void* extra) {
    unused(extra);

    raw_packet_t* raw = event;

    if (raw->iface_class_type != IFACE_CLASS_ETHERNET) {
        return;
    }

    char* iface_name = raw->iface;
    network_iface_t* iface = network_iface_get(iface_name);

    if (iface == NULL) {
        return;
    }

    ethernet_packet_t* eth = (ethernet_packet_t*) raw->buffer;

    if (ntohs(eth->type) != ETH_TYPE_ARP) {
        return;
    }

    arp_state_t* state = get_state(iface);
    arp_packet_t* arp = (arp_packet_t*) eth->payload;

    uint16_t oper = ntohs(arp->oper);
    if (oper == 1) {
        tell(iface, ntohl(arp->sender_ip), arp->sender_ha);
    }

    if (oper == 2) {
        arp_entry_t* entry = hashmap_get(state->table, (void*) (uintptr_t) arp->sender_ip);

        if (entry == NULL) {
            entry = zalloc(sizeof(arp_entry_t));
            hashmap_set(state->table, (void*) (uintptr_t) arp->sender_ip, entry);
        }

        copy_mac(entry->mac, arp->sender_ha);

        dbg(
            "Interface %s was told that " L_IP_FMT " is " L_MAC_FMT "\n",
            iface->name,
            ip_cp(arp->sender_ip),
            mac_cp(entry->mac)
        );
    }
}

static void handle_interface_up(void* event, void* extra) {
    unused(extra);

    network_iface_t* iface = event;
    arp_state_t* state = zalloc(sizeof(arp_state_t));
    state->table = hashmap_create_int(2);
    hashmap_set(iface->manager_data, "arp", state);
}

static void handle_interface_down(void* event, void* extra) {
    unused(extra);

    network_iface_t* iface = event;
    hashmap_remove(iface->manager_data, "arp");
}

void network_stack_arp_init(void) {
    event_add_handler(
        EVENT_NETWORK_STACK_RAW_PKT_RECEIVE,
        handle_potential_arp,
        NULL
    );

    event_add_handler(
        EVENT_NETWORK_STACK_IFACE_UP,
        handle_interface_up,
        NULL
    );

    event_add_handler(
        EVENT_NETWORK_STACK_IFACE_DOWN,
        handle_interface_down,
        NULL
    );

    event_add_handler(
        EVENT_NETWORK_STACK_IFACE_UPDATE,
        handle_config_change,
        NULL
    );
}

void arp_lookup(network_iface_t* iface, uint32_t addr, uint8_t* hw) {
    arp_state_t* state = get_state(iface);
    if (state == NULL) {
        return;
    }

    arp_entry_t* entry = hashmap_get(state->table, (void*) (uintptr_t) addr);
    if (entry == NULL) {
        return;
    }

    copy_mac(hw, entry->mac);
}

list_t* arp_get_known(network_iface_t* iface) {
    arp_state_t* state = get_state(iface);
    if (state == NULL) {
        return NULL;
    }

    return hashmap_keys(state->table);
}

void arp_ask(network_iface_t* iface, uint32_t addr) {
    ask(iface, htonl(addr));
}
