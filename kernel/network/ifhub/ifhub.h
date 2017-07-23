/**
 * Network interface hub service.
 */
#pragma once

#include <kernel/network/iface.h>

typedef struct ifhub_cfg {
    network_iface_t* left;
    network_iface_t* right;
} ifhub_cfg_t;

network_iface_t* ifhub_create(
    char* name,
    network_iface_t* left,
    network_iface_t* right
);
