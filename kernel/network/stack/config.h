#pragma once

#include <stdint.h>

#include <kernel/network/iface.h>
#include <kernel/spin.h>

/* Network interface configuration. */
typedef struct netconf {
    /* IPv4 Configuration. */
    struct {
        /* IPv4 source address. */
        uint32_t source;

        /* IPv4 gateway address. */
        uint32_t gateway;
    } ipv4;

    spin_lock_t lock;
} netconf_t;

netconf_t* netconf_get(network_iface_t*);
void netconf_lock(netconf_t* conf);
void netconf_unlock(netconf_t* conf);

void network_stack_config_init(void);
