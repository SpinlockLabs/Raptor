#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <kernel/network/iface.h>
#include <kernel/network/ip.h>

/**
 * Represents a raw network packet that was received
 * on a network interface.
 */
typedef struct raw_packet {
    /* Name of the iface the packet was received on. */
    char* iface;

    /* The packet ipv4. */
    uint8_t* buffer;

    /**
     * Flag that determines whether we should automatically
     * free the packet buffer.
     */
    bool free;

    /**
     * Flag that determines whether we should automatically
     * re-queue a standard packet event.
     */
    bool translated;

    /**
     * The network interface class type.
     */
    network_iface_class_type_t iface_class_type;
} raw_packet_t;

/**
 * Represents a standard network packet that was received
 * and potentially translated from another format.
 */
typedef struct raw_ipv4_packet {
    /* Name of the iface the packet was received on. */
    char* iface;

    /**
     * The raw packet.
     */
    raw_packet_t* raw;

    /* The packet ipv4. */
    ipv4_packet_t* ipv4;
} raw_ipv4_packet_t;

/* Network stack subsystem initialization. */
void network_stack_init(void);
