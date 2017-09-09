#include "ifhub.h"
#include "log.h"

#include <liblox/printf.h>
#include <liblox/memory.h>
#include <liblox/string.h>

#include <kernel/network/stack/stack.h>

static uint ifhub_id = 0;

static netif_error_t ifhub_destroy(
    netif_t* iface
) {
    ifhub_cfg_t* cfg = iface->data;

    netif_ioctl(
        cfg->left,
        NET_IFACE_IOCTL_DISABLE_PROMISCUOUS,
        NULL
    );

    netif_ioctl(
        cfg->left,
        NET_IFACE_IOCTL_DISABLE_PROMISCUOUS,
        NULL
    );

    cfg->left->manager = NULL;
    cfg->right->manager = NULL;
    cfg->left->handle_receive = NULL;
    cfg->right->handle_receive = NULL;
    cfg->left->manager_data = NULL;
    cfg->right->manager_data = NULL;

    network_stack_takeover_async(cfg->left);
    network_stack_takeover_async(cfg->right);

    free(cfg);

    info(
        "Destroyed %s.\n",
        iface->name
    );

    free(iface->name);
    free(iface);

    return IFACE_ERR_OK;
}

static netif_error_t ifhub_handle_member_receive(
    netif_t* iface,
    uint8_t* buffer,
    size_t size) {
    netif_t* target = iface->manager_data;
    return netif_send(target, buffer, size);
}

netif_t* ifhub_create(
    char* name,
    netif_t* left,
    netif_t* right
) {
    if (left == NULL || right == NULL) {
        return NULL;
    }

    if (name == NULL) {
        name = zalloc(32);
        uint id = ifhub_id++;
        sprintf(name, "ifhub%d", id);
    } else {
        name = strdup(name);
    }

    ifhub_cfg_t* cfg = zalloc(sizeof(ifhub_cfg_t));
    cfg->left = left;
    cfg->right = right;

    network_stack_disown(cfg->left);
    network_stack_disown(cfg->right);

    cfg->left->manager = "ifhub";
    cfg->right->manager = "ifhub";

    cfg->left->manager_data = cfg->right;
    cfg->right->manager_data = cfg->left;

    netif_ioctl(
        cfg->left,
        NET_IFACE_IOCTL_ENABLE_PROMISCUOUS,
        NULL
    );

    netif_ioctl(
        cfg->right,
        NET_IFACE_IOCTL_ENABLE_PROMISCUOUS,
        NULL
    );

    cfg->left->handle_receive = ifhub_handle_member_receive;
    cfg->right->handle_receive = ifhub_handle_member_receive;

    netif_t* hub = netif_create(name);
    hub->class_type = IFACE_CLASS_VIRTUAL;
    hub->flags.stackless = true;
    hub->flags.stub = true;
    hub->data = cfg;
    hub->destroy = ifhub_destroy;
    netif_register(
        device_root(),
        hub
    );

    info(
        "Created %s with %s <-> %s.\n",
        name,
        left->name,
        right->name
    );
    return hub;
}
