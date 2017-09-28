/**
 * Network interface hub service.
 */
#pragma once

#include <kernel/network/iface.h>

typedef struct ifhub_cfg {
    netif_t* left;
    netif_t* right;
} ifhub_cfg_t;

netif_t* ifhub_create(
    char* name,
    netif_t* left,
    netif_t* right
);
