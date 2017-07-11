#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * Represents a raw network packet that was received
 * on a network interface.
 */
typedef struct network_packet {
    /* Name of the interface the packet was received on. */
    char* interface;

    /* The packet buffer. */
    uint8_t* buffer;

    /*
        Flag that determines whether we should automatically free the
        packet buffer.
    */
    bool free;
} network_packet_t;

/* Network stack subsystem initialization. */
void network_stack_init(void);
