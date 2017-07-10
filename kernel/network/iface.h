#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct network_iface network_iface_t;

typedef uint8_t* (*network_iface_get_mac_t)(network_iface_t*);
typedef void (*network_iface_send_t)(network_iface_t*, uint8_t*, size_t size);

struct network_iface {
    char* name;
    network_iface_get_mac_t get_mac;
    network_iface_send_t send;
    void* data;
};

void network_iface_register(network_iface_t* iface);
network_iface_t* network_iface_get(char* name);
void network_iface_subsystem_init(void);
