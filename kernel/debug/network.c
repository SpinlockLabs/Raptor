#include "console.h"

#include <liblox/string.h>

#include <kernel/network/iface.h>
#include <kernel/network/ip.h>

#include <kernel/network/stack/dhcp.h>
#include <kernel/network/stack/arp.h>

#include <kernel/network/ifhub/ifhub.h>

static void debug_network_iface_list(tty_t* tty, const char* input) {
    unused(input);

    list_t* interfaces = netif_get_all();

    list_for_each(node, interfaces) {
        netif_t* iface = node->value;

        uint8_t mac[6];
        netif_get_mac(iface, mac);

        tty_printf(
            tty, "Interface %s:\n", iface->name);

        tty_printf(
            tty,
            "  MAC Address: %2x:%2x:%2x:%2x:%2x:%2x\n",
            mac[0],
            mac[1],
            mac[2],
            mac[3],
            mac[4],
            mac[5]
        );

        tty_printf(
            tty,
            "  Manager: %s\n",
            iface->manager
        );

        int promisc = netif_ioctl(
            iface, NET_IFACE_IOCTL_GET_PROMISCUOUS, NULL);

        if (promisc > 0) {
            tty_printf(tty, "  Promiscuous: true\n");
        } else {
            tty_printf(tty, "  Promiscuous: false\n");
        }
    }
}

static void debug_network_iface_destroy(tty_t* tty, const char* input) {
    netif_t* iface = netif_get((char*) input);
    if (iface == NULL) {
        tty_printf(tty, "Network interface %s was not found.\n", input);
        return;
    }

    netif_destroy(iface);
}

static void debug_network_dhcp_send_request(tty_t* tty, const char* input) {
    netif_t* iface = netif_get((char*) input);
    if (iface == NULL) {
        tty_printf(tty, "Network interface %s was not found.\n", input);
        return;
    }

    dhcp_send_request(iface);
}

static void debug_arp_known(tty_t* tty, const char* input) {
    netif_t* iface = netif_get((char*) input);
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
        uint32_t raddr = (uint32_t) (uintptr_t) node->value;
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

    netif_t* iface = netif_get(iface_name);
    if (iface == NULL) {
        tty_printf(tty, "Network interface %s was not found.\n", iface_name);
        return;
    }
    uint32_t addr = ipv4_address_parse(addr_str);

    arp_ask(iface, addr);
}

static void debug_ifhub_create(tty_t* tty, const char* input) {
    char* rest = (char*) input;
    char* iface_name = NULL;
    char* left_name = NULL;
    char* right_name = NULL;

    char* token = NULL;
    while ((token = strtok(rest, " ", &rest)) != NULL) {
        if (iface_name == NULL) {
            iface_name = token;
        } else if (left_name == NULL) {
            left_name = token;
        } else {
            right_name = token;
            break;
        }
    }

    if (iface_name == NULL ||
        left_name == NULL ||
        right_name == NULL) {
        tty_printf(tty, "Usage: ifhub-create <name> <left> <right>\n");
        return;
    }

    netif_t* left = netif_get(left_name);
    if (left == NULL) {
        tty_printf(tty, "Network interface %s was not found.\n", left_name);
        return;
    }

    netif_t* right = netif_get(right_name);
    if (right == NULL) {
        tty_printf(tty, "Network interface %s was not found.\n", right_name);
        return;
    }

    netif_t* iface = ifhub_create(iface_name, left, right);

    if (iface == NULL) {
        tty_printf(tty, "Failed to create ifhub interface.\n");
        return;
    }

    tty_printf(tty, "Created.\n");
}

void debug_network_init(void) {
    debug_register_command((console_command_t) {
        .name = "net-iface-list",
        .group = "network",
        .help = "List all network interfaces",
        .handler = debug_network_iface_list
    });
    debug_register_command((console_command_t) {
        .name = "net-iface-destroy",
        .group = "network",
        .help = "Destroy a network interface",
        .handler = debug_network_iface_destroy
    });
    debug_register_command((console_command_t) {
        .name = "dhcp-send-request",
        .group = "network",
        .help = "Request a DHCP IP address",
        .handler = debug_network_dhcp_send_request
    });

    debug_register_command((console_command_t) {
        .name = "arp-known",
        .group = "network",
        .help = "List the entire known ARP table",
        .handler = debug_arp_known
    });
    debug_register_command((console_command_t) {
        .name = "arp-ask",
        .group = "network",
        .help = "Ask ARP for a MAC address for given IPv4",
        .handler = debug_arp_ask
    });

    debug_register_command((console_command_t) {
        .name = "ifhub-create",
        .group = "network",
        .help = "Create an if hub",
        .handler = debug_ifhub_create
    });
}
