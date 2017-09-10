#include "mailbox.h"
#include "board.h"

#include "../common/mmio.h"

#include <liblox/memory.h>
#include <liblox/string.h>

#define MAIL_FULL 0x80000000
#define MAIL_EMPTY 0x40000000

#define MAIL0_READ (BOARD_MAILBOX_BASE + 0x00)
#define MAIL0_STATUS (BOARD_MAILBOX_BASE + 0x18)

#define MAIL1_WRITE (BOARD_MAILBOX_BASE + 0x20)
#define MAIL1_STATUS (BOARD_MAILBOX_BASE + 0x38)

#define CODE_REQUEST 0x0
#define CODE_RESPONSE_SUCCESS 0x80000000
#define CODE_RESPONSE_FAILURE 0x80000001

typedef struct bcm_property_buffer {
    uint32_t buffer_size;
    uint32_t code;
    uint8_t tags[];
} bcm_property_buffer_t;

uint32_t bcm_mailbox_read(uint8_t channel) {
    while (true) {
        while (true) {
            uint32_t status = mmio_read(MAIL0_STATUS);
            if ((status & MAIL_EMPTY) == 0) {
                break;
            }
        }
        uint32_t data = mmio_read(MAIL0_READ);
        uint32_t read_chan = data & 0xF;
        if (read_chan == channel) {
            return data & ~0xF;
        }
    }
}

void bcm_mailbox_write(uint8_t channel, uint32_t data) {
    while (true) {
        if ((mmio_read(MAIL1_STATUS) & MAIL_FULL) == 0) {
            break;
        }
    }

    mmio_write(MAIL1_WRITE, channel | data);
}

uint32_t bcm_mailbox_write_read(uint8_t channel, uint32_t data) {
    dmb();
    bcm_mailbox_write(channel, data);
    uint32_t result = bcm_mailbox_read(channel);
    dmb();
    return result;
}

bool bcm_get_property_tag(uint32_t tag_id, void* tag, uint32_t tag_size, uint32_t req_param_size) {
    uint32_t buffer_size = sizeof(bcm_property_buffer_t) + tag_size + sizeof(uint32_t);

    uint8_t byte_buffer[buffer_size + 15];
    bcm_property_buffer_t* buffer = (bcm_property_buffer_t*) (
        ((uint32_t) byte_buffer + 15) & ~15
    );

    buffer->buffer_size = buffer_size;
    buffer->code = CODE_REQUEST;
    memcpy(buffer->tags, tag, tag_size);

    bcm_property_tag_t* header = (bcm_property_tag_t*) buffer->tags;
    header->tag_id = tag_id;
    header->value_buffer_size = tag_size - sizeof(bcm_property_tag_t);
    header->value_size = req_param_size & ~(1 << 31);

    uint32_t* end_tag = (uint32_t*) (buffer->tags + tag_size);
    *end_tag = 0;

    uint32_t buff_addr = BOARD_BUS_ADDRESS((uint32_t) buffer);

    dmb();

    if (bcm_mailbox_write_read(8, buff_addr) != buff_addr) {
        return false;
    }

    dmb();

    if (buffer->code != CODE_RESPONSE_SUCCESS) {
        return false;
    }

    if (!(header->value_size & (1 << 31))) {
        return false;
    }

    header->value_size &= ~(1 << 31);
    if (header->value_size == 0) {
        return false;
    }

    memcpy(tag, buffer->tags, tag_size);
    return true;
}
