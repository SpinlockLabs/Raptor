#include "config.h"

#include <liblox/hashmap.h>
#include <liblox/memory.h>

#include <kernel/dispatch/events.h>

netconf_t* netconf_get(network_iface_t* iface) {
    if (iface == NULL) {
        return NULL;
    }

    if (iface->manager_data == NULL) {
        return NULL;
    }

    return hashmap_get(iface->manager_data, "config");
}

void netconf_lock(netconf_t* conf) {
    spin_lock(&conf->lock);
}

void netconf_unlock(netconf_t* conf) {
    spin_unlock(&conf->lock);
}

static void handle_interface_up(void* event, void* extra) {
    unused(extra);

    network_iface_t* iface = event;
    netconf_t* conf = zalloc(sizeof(netconf_t));
    spin_init(&conf->lock);
    hashmap_set(iface->manager_data, "config", conf);
}

static void handle_interface_down(void* event, void* extra) {
    unused(extra);

    network_iface_t* iface = event;
    netconf_t* conf = netconf_get(iface);
    free(conf);
}

void network_stack_config_init(void) {
    event_add_handler(
        EVENT_NETWORK_STACK_IFACE_UP,
        handle_interface_up,
        NULL
    );

    event_add_handler(
        EVENT_NETWORK_STACK_IFACE_DOWN,
        handle_interface_down,
        NULL
    );
}
