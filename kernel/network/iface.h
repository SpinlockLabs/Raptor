#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <liblox/common.h>
#include <liblox/list.h>

#include <kernel/device/registry.h>

typedef struct netif_flags netif_flags_t;

typedef struct netif netif_t;

/* Network interface subsystem error codes. */
typedef enum netif_error {
    /* Unknown error. */
    IFACE_ERR_UNKNOWN = 0,

    /* Success. */
    IFACE_ERR_OK,

    /* Too much data. */
    IFACE_ERR_TOO_BIG,

    /* Invalid data. */
    IFACE_ERR_INVALID,

    /* Interface does not have a handler for the specified operation. */
    IFACE_ERR_NO_HANDLER,

    /* Invalid interface. */
    IFACE_ERR_BAD_IFACE,

    /* Subsystem not ready. */
    IFACE_ERR_NO_SUBSYS,

    /* Buffer is too full. */
    IFACE_ERR_FULL
} netif_error_t;

/* Interface handler to retrieve the MAC address of the interface. */
typedef uint8_t* (*netif_get_mac_func_t)(netif_t*);

/* Generic handler for a network interface operation that can have an error. */
typedef netif_error_t (*netif_op_func_t)(netif_t*);

/* Interface handler for sending data through a network interface. */
typedef netif_error_t (*netif_send_func_t)(netif_t*, uint8_t*, size_t);

/* Kernel-consumable handler for receiving data from a network interface. */
typedef netif_error_t (*netif_handle_receive_t)(netif_t*, uint8_t*, size_t);

/*
  Generic handler for consuming a potential error from an interface operation.
  The handler should return true if it cares about an error strongly.
*/
typedef bool (*netif_handle_error_t)(netif_t*, netif_error_t);

/*
  Generic handler for consuming multiple interfaces.
  The handler should return true if it wants to stop iteration.
*/
typedef bool (*netif_handle_iter_t)(netif_t*, void*);

/*
  Interface-provided handler for ioctl requests.
*/
typedef int (*netif_handle_ioctl_t)(netif_t*, ulong, void*);

/*
  Network interface flags.
*/
struct netif_flags {
    /* Indicates that this network interface is internal-only. */
    bool internal : 1;

    /* Indicates that this network interface is stackless. */
    bool stackless : 1;

    /* Indicates that this network interface is a stub. */
    bool stub : 1;
};

/* Network interface class types. */
typedef enum netif_class_type {
    /**
     * Unknown network interface class.
     * This can indicate a virtual network interface.
     */
    IFACE_CLASS_UNKNOWN = 0,

    /**
     * Ethernet (wired) network interface class.
     */
    IFACE_CLASS_ETHERNET,

    /**
     * Wireless 802.11 network interface class.
     */
    IFACE_CLASS_WIRELESS_80211,

    /**
     * A virtual network interface.
     */
    IFACE_CLASS_VIRTUAL
} netif_class_type_t;

struct netif {
    /**
     * Network interface name. This should be unique.
     */
    char* name;

    /**
     * Network interface flags.
     */
    union {
        uint32_t _flags;
        netif_flags_t flags;
    };

    /**
     * Class type.
     */
    netif_class_type_t class_type;

    /**
     * Device entry.
     */
    device_entry_t* entry;

    /* Interface-provided handler for getting the interface MAC address. */
    netif_get_mac_func_t get_mac;

    /* Interface-provided handler for sending data through a network interface. */
    netif_send_func_t send;

    /* Interface-provided handler for teardown operations. */
    netif_op_func_t destroy;

    /* Interface-provided handler for ioctl requests. */
    netif_handle_ioctl_t handle_ioctl;

    /* Kernel-consumable handler for receiving data from a network interface. */
    netif_handle_receive_t handle_receive;

    /* Interface-provided user data. */
    void* data;

    /* Manager ID. */
    char* manager;

    /* Pointer to the manager's private data. */
    void* manager_data;
};

/**
 * Allocate a network interface with a given name.
 * The network interface is not automatically registered.
 * @return network interface.
 */
netif_t* netif_create(char*);

/**
 * Registers a network interface with the subsystem.
 * @param parent parent device
 * @param iface network interface
 */
void netif_register(
    device_entry_t* parent,
    netif_t* iface
);

/*
  Retrieves a network interface by name.
  Returns a NULL pointer if there is not an interface with that name.
*/
netif_t* netif_get(char*);

/*
  Retrieves a list of all the available network interfaces.
  Returns a new list for each call, and the list must be freed manually.
*/
list_t* netif_get_all(void);

/**
 * Destroys the given network interface.
 * @param iface network interface
 * @return error code
 */
netif_error_t netif_destroy(netif_t* iface);

/**
 * Sends the given buffer through the given network interface.
 * @param iface network interface
 * @param buffer data buffer
 * @param size buffer size
 * @return error code
 */
netif_error_t netif_send(netif_t* iface, uint8_t* buffer, size_t size);

/**
 * Gets the MAC address for the given network interface.
 * @param iface network interface
 * @param mac a six-byte output array
 * @return error code
 */
netif_error_t netif_get_mac(netif_t* iface, uint8_t* mac);

/**
 * Runs an ioctl request on a network interface.
 * @param iface network interface
 * @param request request id
 * @param data request data
 * @return ioctl result
 */
int netif_ioctl(netif_t* iface, ulong request, void* data);

/**
 * Iterates over each network interface and calls the given handler.
 * The given user data is passed into the iteration handler.
 * If the iteration handler returns true, then iteration is stopped.
 * @param func function
 * @param extra extra data
 */
void netif_each(netif_handle_iter_t func, void* extra);

/**
 * Initializes the network interface subsystem.
 */
void netif_subsystem_init(void);

/**
 * ioctl to get the promiscuous boolean flag.
 */
#define NET_IFACE_IOCTL_GET_PROMISCUOUS 0xEF420

/**
 * ioctl to turn on the promiscuous network interface option.
 */
#define NET_IFACE_IOCTL_ENABLE_PROMISCUOUS 0xEF421

/**
 * ioctl to turn off the promiscuous network interface option.
 */
#define NET_IFACE_IOCTL_DISABLE_PROMISCUOUS 0xEF422
