#include "console.h"

#include <kernel/network/iface.h>
#include <kernel/network/stack/dhcp.h>
#include <kernel/network/stack/arp.h>
#include <kernel/network/ip.h>
#include <liblox/string.h>

static void debug_network_iface_list(tty_t* tty, const char* input) {
    unused(input);

    list_t* interfaces = network_iface_get_all();

    list_for_each(node, interfaces) {
        network_iface_t* iface = node->value;

        uint8_t mac[6];
        network_iface_get_mac(iface, mac);

        tty_printf(tty, "Interface %s:\n", iface->name);

        tty_printf(tty,
                   "  MAC Address: %2x:%2x:%2x:%2x:%2x:%2x\n",
                   mac[0],
                   mac[1],
                   mac[2],
                   mac[3],
                   mac[4],
                   mac[5]
        );
    }
}

static void debug_network_iface_destroy(tty_t* tty, const char* input) {
    network_iface_t* iface = network_iface_get((char*) input);
    if (iface == NULL) {
        tty_printf(tty, "Network interface %s was not found.\n", input);
        return;
    }

    network_iface_destroy(iface);
}

static void debug_network_dhcp_send_request(tty_t* tty, const char* input) {
    network_iface_t* iface = network_iface_get((char*) input);
    if (iface == NULL) {
        tty_printf(tty, "Network interface %s was not found.\n", input);
        return;
    }

    dhcp_send_request(iface);
}

static void debug_arp_known(tty_t* tty, const char* input) {
    network_iface_t* iface = network_iface_get((char*) input);
    if (iface == NULL) {
        tty_printf(tty, "Network interface %s was not found.\n", input);
        return;
    }

    list_t* list = arp_get_known(iface);
    if (list == NULL) {
        tty_printf(tty, "ARP does not know any hosts on %s\n", input);
        return;
    }

    list_for_each(node, list) {
        uint32_t raddr = (uint32_t) node->value;
        ipv4_address_t* addr = (ipv4_address_t*) &raddr;
        uint8_t hw[6] = {0};
        arp_lookup(iface, addr->address, hw);
        tty_printf(
            tty,
            "%d.%d.%d.%d = %2x:%2x:%2x:%2x:%2x:%2x\n",
            addr->a,
            addr->b,
            addr->c,
            addr->d,
            hw[0],
            hw[1],
            hw[2],
            hw[3],
            hw[4],
            hw[5]
        );
    }

    list_free(list);
}

static void debug_arp_ask(tty_t* tty, const char* input) {
    char* rest = (char*) input;
    uint idx = 0;

    char* iface_name = NULL;
    char* addr_str = NULL;

    char* token = NULL;
    while ((token = strtok(rest, " ", &rest)) != NULL) {
        if (iface_name == NULL) {
            iface_name = token;
        } else {
            addr_str = token;
            break;
        }
    }

    if (iface_name == NULL || addr_str == NULL) {
        tty_printf(tty, "Usage: arp-ask <interface> <ipv4>\n");
        return;
    }

    network_iface_t* iface = network_iface_get(iface_name);
    if (iface == NULL) {
        tty_printf(tty, "Network interface %s was not found.\n", iface_name);
        return;
    }
    uint32_t addr = ipv4_address_parse(addr_str);

    arp_ask(iface, addr);
}

void debug_network_init(void) {
    debug_console_register_command("net-iface-list", debug_network_iface_list);
    debug_console_register_command("net-iface-destroy", debug_network_iface_destroy);
    debug_console_register_command("dhcp-send-request", debug_network_dhcp_send_request);
    debug_console_register_command("arp-known", debug_arp_known);
    debug_console_register_command("arp-ask", debug_arp_ask);
}
