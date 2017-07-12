#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <kernel/network/iface.h>

typedef struct dhcp_internal_state {
    /* Flag to check if we have accepted an offer. */
    bool accepted;

    /* The last IP offer we received. */
    uint32_t offer;
} dhcp_internal_state_t;

void network_stack_dhcp_init(void);

/**
 * Sends a DHCP request on the given network interface.
 */
void dhcp_send_request(network_iface_t*);
