#include <liblox/io.h>
#include <liblox/string.h>
#include <liblox/memory.h>

#include <kernel/network/iface.h>

#ifdef USE_NPCAP
#include "C:/npcap-sdk/Include/Packet32.h"
#include <ntddndis.h>

char AdapterList[32][1024] = {0};
uint AdapterCount = 0;

struct npcap_adapter {
    netif_t* iface;
    uint8_t mac[6];
};

static uint8_t* npcap_get_mac(netif_t* iface) {
    struct npcap_adapter* a = iface->data;
    return a->mac;
}

void npcap_add_adapter(char* id) {
    LPADAPTER adapter = PacketOpenAdapter(id);
    if (adapter == NULL) {
        printf(WARN "[NPCAP] Failed to open adapter '%s'\n", id);
        return;
    }
    struct npcap_adapter* a = zalloc(sizeof(struct npcap_adapter));
    netif_t* iface = netif_create(id);
    a->iface = iface;
    iface->data = a;
    iface->get_mac = npcap_get_mac;

    {
        PPACKET_OID_DATA oid_data = zalloc(6 + sizeof(PACKET_OID_DATA));
        oid_data->Oid = OID_802_3_CURRENT_ADDRESS;
        oid_data->Length = 6;
        memset(oid_data->Data, 0, 6);
        if (PacketRequest(adapter, false, oid_data) != 0) {
            memcpy(a->mac, oid_data->Data, 6);
        }
        free(oid_data);
    }

    netif_register(device_root(), iface);
}

void raptor_user_network_init(void) {
    char AdapterName[8192];
    ulong AdapterLength = sizeof(AdapterName);

    if (PacketGetAdapterNames(AdapterName, &AdapterLength) == 0) {
        printf(WARN "[NPCAP] Failed to get adapter names.\n");
        return;
    }

    char* temp = AdapterName;
    char* temp1 = AdapterName;

    uint i = 0;
    while (*temp != '\0' || *(temp - 1) != '\0') {
        if (*temp == '\0') {
            memcpy(AdapterList[i], temp1, temp - temp1);
            temp1 = temp + 1;
            i++;
        }
        temp++;
    }
    AdapterCount = i;

    for (uint x = 0; x < AdapterCount; x++) {
        npcap_add_adapter(AdapterList[x]);
    }
}
#else
void raptor_user_network_init(void) {}
#endif
