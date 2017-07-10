#include "iface.h"

#include <liblox/hashmap.h>

static hashmap_t* network_iface_subsystem_registry = NULL;

void network_iface_register(network_iface_t* iface) {
    if (network_iface_subsystem_registry == NULL) {
        return;
    }

    hashmap_set(network_iface_subsystem_registry, iface->name, iface);
}

network_iface_t* network_iface_get(char* name) {
    return hashmap_get(network_iface_subsystem_registry, name);
}

void network_iface_subsystem_init(void) {
    network_iface_subsystem_registry = hashmap_create(10);
}
