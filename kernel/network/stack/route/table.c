#include "table.h"

#include <liblox/io.h>
#include <liblox/string.h>

#include <kernel/panic.h>

#define dbg(msg, ...) printf(DEBUG msg, ##_VA_ARGS__)

/**
 * Why a list when tables are named?
 * Our hashmap is unordered, and routing tables
 * are ordered, therefore we must use a list to
 * maintain the tables.
 */
static list_t* tables = NULL;

static spin_lock_t tables_lock = {0};

static void init_root_tables(void) {
    route_table_t* table = route_table_create("");
    route_error_t error = route_table_add(table);
    if (error != ROUTE_ERROR_OK) {
        panic("Failed to register root routing tables.");
    }
}

void network_stack_route_tables_init(void) {
    tables = list_create();
    init_root_tables();
}

route_table_t* route_table_create(char* name) {
    size_t name_size = strlen(name);

    if (name_size > ROUTE_TABLE_MAX_NAME_SIZE) {
        return NULL;
    }

    route_table_t* table = zalloc(sizeof(route_table_t));
    memcpy(table->name, name, name_size);
    spin_init(table->lock);
    return table;
}

route_table_t* route_table_get(char* name) {
    spin_lock(tables_lock);

    list_for_each(node, tables) {
        route_table_t* table = node->value;

        if (strcmp(table->name, name) == 0) {
            spin_unlock(tables_lock);
            return table;
        }
    }

    spin_unlock(tables_lock);
    return NULL;
}

route_error_t route_table_add(route_table_t* table) {
    route_table_t* existing = route_table_get(table->name);
    if (existing != NULL) {
        return ROUTE_ERROR_EXISTS;
    }

    spin_lock(tables_lock);
    list_add(tables, table);
    spin_unlock(tables_lock);

    return ROUTE_ERROR_OK;
}

route_error_t route_table_remove(route_table_t* table) {
    route_table_t* existing = route_table_get(table->name);
    if (existing == NULL) {
        return ROUTE_ERROR_DOES_NOT_EXIST;
    }

    spin_lock(tables_lock);
    list_remove(list_find(tables, existing));
    spin_unlock(tables_lock);

    return ROUTE_ERROR_OK;
}

list_t* route_table_get_all(void) {
    spin_lock(tables_lock);
    list_t* list = list_create();
    list_for_each(node, tables) {
        list_add(list, node->value);
    }
    spin_unlock(tables_lock);
    return list;
}

static bool does_attribute_match(route_attribute_t* attr, ip_packet_moving_t* packet) {
    if (attr->type == ROUTE_ATTR_V4_SOURCE) {
        return ipv4_cidr_match(
            &attr->v4_source,
            (ipv4_address_t*) &packet->ipv4.source
        );
    }

    if (attr->type == ROUTE_ATTR_V4_DESTINATION) {
        return ipv4_cidr_match(
            &attr->v4_destination,
            (ipv4_address_t*) &packet->ipv4.destination
        );
    }

    if (attr->type == ROUTE_ATTR_TARGET_DEVICE) {
        if (packet->iface == NULL) {
            return attr->target_device == NULL;
        }
        return strcmp(attr->target_device, packet->iface) == 0;
    }

    return false;
}

static void apply_attribute(route_attribute_t* attr, ip_packet_moving_t* packet) {
    if (attr->type == ROUTE_ATTR_TARGET_DEVICE) {
        packet->iface = attr->target_device;
    }
}

route_error_t route_packet(ip_packet_moving_t* packet) {
    unused(packet);

    spin_lock(tables_lock);

    list_for_each(tnode, tables) {
        route_table_t* table = tnode->value;

        spin_lock(table->lock);
        list_for_each(node, table->entries) {
            /* TODO(kaendfinger): Implement route matching. */
        }
        spin_unlock(table->lock);
    }

    spin_unlock(tables_lock);

    return ROUTE_ERROR_OK;
}
