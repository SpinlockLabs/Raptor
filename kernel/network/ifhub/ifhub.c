#include "ifhub.h"
#include "log.h"

#include <liblox/printf.h>
#include <liblox/string.h>

#include <kernel/network/stack/stack.h>

static uint ifhub_id = 0;

static network_iface_error_t ifhub_destroy(
    network_iface_t* iface
) {
    ifhub_cfg_t* cfg = iface->data;

    network_stack_takeover(cfg->left);
    network_stack_takeover(cfg->right);

    free(cfg);

    info(
        "Destroyed %s.\n",
        iface->name
    );

    free(iface->name);

    return IFACE_ERR_OK;
}

network_iface_t* ifhub_create(
    char* name,
    network_iface_t* left,
    network_iface_t* right
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

    cfg->left->handle_receive = cfg->right->send;
    cfg->right->handle_receive = cfg->left->send;

    network_iface_t* hub = network_iface_create(name);
    hub->class_type = IFACE_CLASS_VIRTUAL;
    hub->flags.stackless = true;
    hub->flags.stub = true;
    hub->data = cfg;
    hub->destroy = ifhub_destroy;
    network_iface_register(hub);

    info(
        "Created %s with %s <-> %s.\n",
        name,
        left->name,
        right->name
    );
    return hub;
}
