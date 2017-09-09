#pragma once

#include <stdint.h>
#include <stdbool.h>

uint32_t bcm_mailbox_read(uint8_t channel);
void bcm_mailbox_write(uint8_t channel, uint32_t data);
uint32_t bcm_mailbox_write_read(uint8_t channel, uint32_t data);

bool bcm_get_property_tag(uint32_t tag_id, void* tag, uint32_t tag_size, uint32_t req_param_size);

typedef struct bcm_property_tag {
    uint32_t tag_id;
    uint32_t value_buffer_size;
    uint32_t value_size;
} bcm_property_tag_t;

typedef struct bcm_property_mac {
    bcm_property_tag_t tag;
    uint8_t address[6];
    uint8_t padding[2];
} bcm_property_mac_t;

typedef struct bcm_property_power {
    bcm_property_tag_t tag;
    uint32_t device_id;
    uint32_t state;
} bcm_property_power_t;

#define BCM_DEVICE_ID_SD_CARD 0
#define BCM_DEVICE_ID_USB_HCD 3

#define BCM_POWER_STATE_OFF (0 << 0)
#define BCM_POWER_STATE_ON (1 << 0)
#define BCM_POWER_STATE_WAIT (1 << 1)
#define BCM_POWER_STATE_NO_DEVICE (1 << 1)

#define BCM_TAG_GET_FIRMWARE_REVISION	0x00000001
#define BCM_TAG_GET_BOARD_MODEL		0x00010001
#define BCM_TAG_GET_BOARD_REVISION	0x00010002
#define BCM_TAG_GET_MAC_ADDRESS		0x00010003
#define BCM_TAG_GET_BOARD_SERIAL	0x00010004
#define BCM_TAG_GET_ARM_MEMORY		0x00010005
#define BCM_TAG_GET_VC_MEMORY		0x00010006
#define BCM_TAG_SET_POWER_STATE		0x00028001
#define BCM_TAG_GET_CLOCK_RATE		0x00030002
#define BCM_TAG_GET_TEMPERATURE		0x00030006
#define BCM_TAG_GET_EDID_BLOCK		0x00030020
#define BCM_TAG_GET_DISPLAY_DIMENSIONS	0x00040003
#define BCM_TAG_GET_COMMAND_LINE	0x00050001
