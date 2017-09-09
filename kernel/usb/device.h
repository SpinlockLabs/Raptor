#pragma once

#include "protocol/desc.h"

typedef struct usb_endpoint {
    usb_endpoint_desc_t desc;
    uint32_t toggle;
} usb_endpoint_t;

typedef struct usb_device_req {
    uint8_t type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} usb_device_req_t;

typedef struct usb_transfer {
    usb_endpoint_t* endpoint;
    usb_device_req_t* req;
    void* data;
    uint32_t length;

    union {
        bool complete;
        bool success;

        uint32_t _flags;
    };
} usb_transfer_t;
