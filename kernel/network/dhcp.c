#include "dhcp.h"

#include <liblox/net.h>
#include <liblox/string.h>

void dhcp_create_generic_request(network_iface_t* iface, dhcp_packet_t* dhcp) {
    uint8_t mac[6];
    network_iface_get_mac(iface, mac);

    dhcp->op = 1;
    dhcp->htype = 1;
    dhcp->hlen = 6;
    dhcp->hops = 0;
    dhcp->xid = htonl(0x1337);
    dhcp->secs = 0;
    dhcp->flags = 0;
    dhcp->ciaddr = 0;
    dhcp->yiaddr = 0;
    dhcp->siaddr = 0;
    dhcp->giaddr = 0;

    memset(&dhcp->chaddr, 0, 16);
    for (int i = 0; i < 6; i++) {
        dhcp->chaddr[i] = mac[i];
    }

    memset(&dhcp->sname, 0, 64);
    memset(&dhcp->file, 0, 128);
    dhcp->magic = htonl(DHCP_MAGIC);
}
