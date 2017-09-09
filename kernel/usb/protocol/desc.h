#pragma once

#include <liblox/common.h>

// USB Base Descriptor Types
#define USB_DESC_DEVICE                 0x01
#define USB_DESC_CONF                   0x02
#define USB_DESC_STRING                 0x03
#define USB_DESC_INTF                   0x04
#define USB_DESC_ENDP                   0x05

// USB HID Descriptor Types

#define USB_DESC_HID                    0x21
#define USB_DESC_REPORT                 0x22
#define USB_DESC_PHYSICAL               0x23

// USB HUB Descriptor Types

#define USB_DESC_HUB                    0x29

typedef struct usb_device_desc {
    uint8_t length;
    uint8_t type;
    uint16_t usb_version;
    uint8_t device_class;
    uint8_t device_subclass;
    uint8_t protocol;
    uint8_t max_packet_size;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t device_version;
    uint8_t vendor_str;
    uint8_t product_str;
    uint8_t serial_str;
    uint8_t conf_count;
} packed usb_device_desc_t;

typedef struct usb_conf_desc {
    uint8_t length;
    uint8_t type;
    uint16_t total_length;
    uint8_t interface_count;
    uint8_t conf_value;
    uint8_t conf_str;
    uint8_t attributes;
    uint8_t max_power;
} packed usb_conf_desc_t;

typedef struct usb_string_desc {
    uint8_t length;
    uint8_t type;
    uint16_t str[];
} packed usb_string_desc_t;

typedef struct usb_iface_desc {
    uint8_t length;
    uint8_t type;
    uint8_t interface_index;
    uint8_t alt_setting;
    uint8_t endpoint_count;
    uint8_t interface_class;
    uint8_t interface_subclass;
    uint8_t interface_protocol;
    uint8_t interface_str;
} packed usb_iface_desc_t;

typedef struct usb_endpoint_desc {
    uint8_t length;
    uint8_t type;
    uint8_t address;
    uint8_t attributes;
    uint16_t max_packet_size;
    uint8_t interval;
} packed usb_endpoint_desc_t;

typedef struct usb_hid_desc {
    uint8_t length;
    uint8_t type;
    uint16_t hid_version;
    uint8_t country_code;
    uint8_t desc_count;
    uint8_t desc_type;
    uint8_t desc_len;
} packed usb_hid_desc_t;

typedef struct usb_hub_desc {
    uint8_t length;
    uint8_t type;
    uint8_t port_count;
    uint16_t chars;
    uint8_t port_power_time;
    uint8_t current;
} packed usb_hub_desc_t;

// Hub Characteristics
#define HUB_POWER_MASK                  0x03        // Logical Power Switching Mode
#define HUB_POWER_GLOBAL                0x00
#define HUB_POWER_INDIVIDUAL            0x01
#define HUB_COMPOUND                    0x04        // Part of a Compound Device
#define HUB_CURRENT_MASK                0x18        // Over-current Protection Mode
#define HUB_TT_TTI_MASK                 0x60        // TT Think Time
#define HUB_PORT_INDICATORS             0x80        // Port Indicators
