#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <liblox/common.h>
#include <liblox/list.h>

typedef struct network_iface_flags network_iface_flags_t;

typedef struct network_iface network_iface_t;

/* Network interface subsystem error codes. */
typedef enum network_iface_error {
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
} network_iface_error_t;

/* Interface handler to retrieve the MAC address of the interface. */
typedef uint8_t* (*network_iface_get_mac_func_t)(network_iface_t*);

/* Generic handler for a network interface operation that can have an error. */
typedef network_iface_error_t (*network_iface_op_func_t)(network_iface_t*);

/* Interface handler for sending data through a network interface. */
typedef network_iface_error_t (*network_iface_send_func_t)(network_iface_t*, uint8_t*, size_t);

/* Kernel-consumable handler for receiving data from a network interface. */
typedef void (*network_iface_handle_receive_t)(network_iface_t*, uint8_t*);

/*
  Generic handler for consuming a potential error from an interface operation.
  The handler should return true if it cares about an error strongly.
*/
typedef bool (*network_iface_handle_error_t)(network_iface_t*, network_iface_error_t);

/*
  Generic handler for consuming multiple interfaces.
  The handler should return true if it wants to stop iteration.
*/
typedef bool (*network_iface_handle_iter_t)(network_iface_t*, void*);

/*
  Interface-provided handler for ioctl requests.
*/
typedef int (*network_iface_handle_ioctl_t)(network_iface_t*, ulong, void*);

/*
  Network interface flags.
*/
struct network_iface_flags {
    /* Indicates that this network interface is internal-only. */
    bool internal : 1;
};

/* Network interface class types. */
typedef enum network_iface_class_type {
    /*
      Unknown network interface class.
      This can indicate a virtual network interface.
    */
    IFACE_CLASS_UNKNOWN = 0,

    /*
      Ethernet (wired) network interface class.
    */
    IFACE_CLASS_ETHERNET,

    /*
      Wireless 802.11 network interface class.
    */
    IFACE_CLASS_WIRELESS_80211
} network_iface_class_type_t;

struct network_iface {
    /*
      Network interface name. This should be unique.
    */
    char* name;

    /*
      Network interface flags.
    */
    union {
        uint32_t _flags;
        network_iface_flags_t flags;
    };

    /*
      Class type.
    */
    network_iface_class_type_t class_type;

    /* Interface-provided handler for getting the interface MAC address. */
    network_iface_get_mac_func_t get_mac;

    /* Interface-provided handler for sending data through a network interface. */
    network_iface_send_func_t send;

    /* Interface-provided handler for teardown operations. */
    network_iface_op_func_t destroy;

    /* Interface-provided handler for ioctl requests. */
    network_iface_handle_ioctl_t handle_ioctl;

    /* Kernel-consumable handler for receiving data from a network interface. */
    network_iface_handle_receive_t handle_receive;

    /* Interface-provided user data. */
    void* data;

    /* Pointer to network stack private data. */
    void* stack;
};

/*
  Allocate a network interface with a given name.
  The network interface is not automatically registered.
*/
network_iface_t* network_iface_create(char*);

/* Registers a network interface with the subsystem. */
void network_iface_register(network_iface_t*);

/*
  Retrieves a network interface by name.
  Returns a NULL pointer if there is not an interface with that name.
*/
network_iface_t* network_iface_get(char*);

/*
  Retrieves a list of all the available network interfaces.
  Returns a new list for each call, and the list must be freed manually.
*/
list_t* network_iface_get_all(void);

/*
  Destroys the given network interface.
*/
network_iface_error_t network_iface_destroy(network_iface_t*);

/*
  Sends the given buffer through the given network interface.
*/
network_iface_error_t network_iface_send(network_iface_t*, uint8_t*, size_t);

/*
  Gets the MAC address for the given network interface.
*/
network_iface_error_t network_iface_get_mac(network_iface_t*, uint8_t[6]);

/*
  Runs an ioctl request on a network interface.
*/
int network_iface_ioctl(network_iface_t*, ulong, void*);

/*
  Iterates over each network interface and calls the given handler.
  The given user data is passed into the iteration handler.
  If the iteration handler returns true, then iteration is stopped.
*/
void network_iface_each(network_iface_handle_iter_t, void*);

/* Initializes the network interface subsystem. */
void network_iface_subsystem_init(void);
