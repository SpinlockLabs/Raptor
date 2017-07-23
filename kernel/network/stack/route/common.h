#pragma once

#include <kernel/network/ip.h>

/* A routing error. */
typedef enum route_error {
    ROUTE_ERROR_UNKNOWN = 0,
    ROUTE_ERROR_OK,
    ROUTE_ERROR_NO_ROUTE,
    ROUTE_ERROR_EXISTS,
    ROUTE_ERROR_DOES_NOT_EXIST
} route_error_t;

/* Types of routing attributes. */
typedef enum route_attribute_type {
    ROUTE_ATTR_V4_SOURCE,
    ROUTE_ATTR_V4_DESTINATION,
    ROUTE_ATTR_TARGET_DEVICE
} route_attribute_type_t;

/* A routing attribute. */
typedef struct route_attribute {
    route_attribute_type_t type;
    union {
        ipv4_address_cidr_t v4_source;
        ipv4_address_cidr_t v4_destination;
        char* target_device;
    };
} route_attribute_t;
