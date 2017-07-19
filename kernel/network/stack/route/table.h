#pragma once

#include "common.h"

#include <liblox/list.h>

#include <kernel/spin.h>

#define ROUTE_TABLE_MAX_NAME_SIZE 63

/* Types of routing entries. */
typedef enum route_entry_type {
    /* Match all of the given attributes. */
    ROUTE_ENTRY_TYPE_MATCH_ALL = 1
} route_entry_type_t;

/* A routing table entry. */
typedef struct route_table_entry {
    /* Condition type. */
    route_entry_type_t type;

    /* Number of criteria. */
    size_t criteria_size;

    /* Criteria. */
    route_attribute_t* criteria;

    /* Number of modifiers. */
    size_t modifier_size;

    /* Modifiers. */
    route_attribute_t* modifiers;

    /* Owner storage. */
    void* owner;
} route_table_entry_t;

/* A routing table. */
typedef struct route_table {
    /* Name of the routing table. */
    char name[ROUTE_TABLE_MAX_NAME_SIZE + 1];

    /* Table entry list. */
    list_t* entries;

    /* Table lock. */
    spin_lock_t lock;
} route_table_t;

/* Initializes the routing table layer. */
void network_stack_route_tables_init(void);

/* Creates a routing table with the given name. */
route_table_t* route_table_create(char* name);

/* Add a route table to the routing table layer. */
route_error_t route_table_add(route_table_t* table);

/* Remove a route table from the routing table layer. */
route_error_t route_table_remove(route_table_t* table);

/* Get a route table by name. */
route_table_t* route_table_get(char* name);

/* Get all routing tables. */
list_t* route_table_get_all(void);

/* Route a packet. */
route_error_t route_packet(ip_packet_moving_t*);
