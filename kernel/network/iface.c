#include "iface.h"

#include <liblox/string.h>
#include <liblox/memory.h>
#include <liblox/hashmap.h>

#include <kernel/panic.h>
#include <kernel/spin.h>

#include <kernel/dispatch/events.h>

static spin_lock_t network_subsystem_lock;
static hashmap_t* network_iface_subsystem_registry = NULL;

static inline void ensure_subsystem(void) {
    if (network_iface_subsystem_registry == NULL) {
        panic("Network interface subsystem has not been initialized.");
    }
}

void netif_register(
    device_entry_t* parent,
    netif_t* iface
) {
    ensure_subsystem();
    spin_lock(&network_subsystem_lock);
    hashmap_set(network_iface_subsystem_registry, iface->name, iface);
    spin_unlock(&network_subsystem_lock);

    event_dispatch(EVENT_NETWORK_IFACE_REGISTERED, iface->name);

    iface->entry = device_register(
        parent,
        iface->name,
        DEVICE_CLASS_NETWORK,
        iface
    );
}

list_t* netif_get_all(void) {
    ensure_subsystem();
    return hashmap_values(network_iface_subsystem_registry);
}

netif_error_t netif_destroy(netif_t* iface) {
    if (network_iface_subsystem_registry == NULL) {
        return IFACE_ERR_NO_SUBSYS;
    }

    if (iface == NULL) {
        return IFACE_ERR_BAD_IFACE;
    }

    event_dispatch(EVENT_NETWORK_IFACE_DESTROYING, iface);

    device_unregister(iface->entry);

    spin_lock(&network_subsystem_lock);

    netif_error_t error = IFACE_ERR_OK;

    char* name = strdup(iface->name);
    if (iface->destroy != NULL) {
        error = iface->destroy(iface);
    }

    if (error == IFACE_ERR_OK) {
        hashmap_remove(network_iface_subsystem_registry, name);
    }

    spin_unlock(&network_subsystem_lock);
    event_dispatch(EVENT_NETWORK_IFACE_DESTROYED, name);
    free(name);

    return error;
}

netif_error_t netif_get_mac(netif_t* iface, uint8_t* mac) {
    if (iface == NULL) {
        return IFACE_ERR_BAD_IFACE;
    }

    if (iface->get_mac == NULL) {
        return IFACE_ERR_NO_HANDLER;
    }

    uint8_t* m = iface->get_mac(iface);
    if (m == NULL) {
        return IFACE_ERR_UNKNOWN;
    }

    memcpy(mac, m, 6);
    return IFACE_ERR_OK;
}

netif_t* netif_create(char* name) {
    ensure_subsystem();

    netif_t* iface = zalloc(sizeof(netif_t));
    ensure_allocated(iface);
    iface->name = name;
    return iface;
}

netif_t* netif_get(char* name) {
    ensure_subsystem();

    spin_lock(&network_subsystem_lock);
    netif_t* iface = hashmap_get(network_iface_subsystem_registry, name);
    spin_unlock(&network_subsystem_lock);
    return iface;
}

void netif_each(netif_handle_iter_t handle, void* data) {
    ensure_subsystem();

    spin_lock(&network_subsystem_lock);

    bool end = false;
    for (uint i = 0; i < network_iface_subsystem_registry->size; ++i) {
        hashmap_entry_t* x = network_iface_subsystem_registry->entries[i];
        while (x) {
            if (handle(x->value, data)) {
                end = true;
                break;
            }

            x = x->next;
        }

        if (end) {
            break;
        }
    }

    spin_unlock(&network_subsystem_lock);
}

netif_error_t netif_send(netif_t* iface,
                         uint8_t* buffer, size_t size) {
    if (iface == NULL) {
        return IFACE_ERR_BAD_IFACE;
    }

    if (iface->send != NULL) {
        return iface->send(iface, buffer, size);
    }
    return IFACE_ERR_NO_HANDLER;
}

int netif_ioctl(netif_t* iface, ulong request, void* data) {
    if (iface == NULL) {
        return -IFACE_ERR_BAD_IFACE;
    }

    if (iface->handle_ioctl == NULL) {
        return -IFACE_ERR_NO_HANDLER;
    }

    return iface->handle_ioctl(iface, request, data);
}

void netif_subsystem_init(void) {
    network_iface_subsystem_registry = hashmap_create(10);
    spin_init(&network_subsystem_lock);
    event_dispatch(EVENT_NETWORK_IFACE_SUBSYSTEM_INIT, NULL);
}
