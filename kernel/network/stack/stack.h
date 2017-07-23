#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <kernel/network/iface.h>
#include <kernel/network/ip.h>

/* The direction of a packet in motion through the stack. */
typedef enum packet_direction {
    /* Network Interface -> Network Stack */
    PACKET_DIRECTION_IN = 1,
    /* Network Stack -> Network Interface */
    PACKET_DIRECTION_OUT = 2
} packet_direction_t;

/**
 * A packet classification.
 */
typedef enum packet_class {
    /**
     * An unknown packet class. A packet translation layer
     * might choose to ignore packets with this class due to
     * a lack of link-layer protocol information.
     */
    PACKET_CLASS_UNKNOWN = 0,

    /**
     * IPv4 packet class.
     */
    PACKET_CLASS_IPV4 = 1,

    /**
     * ARP packet class.
     */
    PACKET_CLASS_ARP = 2
} packet_class_t;

/**
 * Represents either a raw network packet that was received
 * or a request to send a raw network packet through an interface.
 */
typedef struct raw_packet {
    /**
     * The direction the packet is traveling.
     * Incoming means that the packet was received from an interface.
     * Outgoing means that the packet is going to an interface.
     */
    packet_direction_t direction;

    /**
     * Incoming: Name of the interface the packet was received on.
     * Outgoing: Name of the interface to send the packet through.
     */
    char* iface;

    /**
     * A non-translated packet buffer.
     */
    uint8_t* buffer;

    /**
     * Flag that determines whether we should automatically
     * free the packet buffer after dispatching is complete.
     */
    bool free;

    /**
     * Incoming: Flag that determines whether we should automatically
     * re-queue a standard packet event.
     * Outgoing: Flag that determines whether to deliver the packet straight
     * to the network interface after dispatching is complete.
     */
    bool translated;

    /**
     * Incoming: If the length of the packet is known, this field will contain
     * the size of the packet buffer.
     * Outgoing: The size of the outgoing packet buffer.
     */
    uint16_t length;

    /**
     * Incoming: The received packet's classification, if known.
     * Outgoing: The outgoing packet's classification. Some link-layer
     * translators may choose to ignore packets that do not set this to
     * a correct value.
     */
    packet_class_t packet_class;

    /**
     * Incoming: Will generally be all unset, but could be utilized for
     * future, unforeseen use cases.
     * Outgoing: Additional flags to pass through the stack.
     */
    uint32_t flags;

    /**
     * The relevant network interface class type.
     */
    network_iface_class_type_t iface_class_type;
} raw_packet_t;

/**
 * Represents an IPv4 network packet that was received
 * and potentially translated from another format.
 */
typedef struct raw_ipv4_packet {
    /* Name of the interface the packet was received on. */
    char* iface;

    /**
     * The raw packet.
     */
    raw_packet_t* raw;

    /* The IPv4 packet. */
    ipv4_packet_t* ipv4;
} raw_ipv4_packet_t;

/* Network stack subsystem initialization. */
void network_stack_init(void);

/* Lets the network stack takeover the given interface. */
bool network_stack_takeover(network_iface_t* iface);

/* Lets the network stack takeover the given interface. */
void network_stack_takeover_async(network_iface_t* iface);

/* Lets the network stack disown the given interface. */
bool network_stack_disown(network_iface_t* iface);

/**
 * Triggers the network stack to send the given buffer through the
 * given network interface. This will potentially translate the packet
 * into the correct link-level packet format.
 * The buffer will be automatically freed by the network stack.
 * The provided classifier can be used by the packet translation layers
 * to assign the correct link-layer protocol classifiers.
 * The provided flags are also passed through the packet translation layers
 * to allow further specifics if necessary.
 */
void network_stack_send_packet(
    network_iface_t* iface,
    uint8_t* buffer,
    size_t size,
    packet_class_t packet_class,
    uint32_t flags);
