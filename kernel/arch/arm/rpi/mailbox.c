#include "mailbox.h"

#define READ_STATUS_OFFSET 0x18
#define MAIL_FULL 0x80000000
#define MAIL_EMPTY 0x40000000
#define MAIL_STATUS (BOARD_MAILBOX_BASE + READ_STATUS_OFFSET)
#define MAIL_WRITE (BOARD_MAILBOX_BASE + 0x20)

uint32_t bcm_mailbox_read(uint8_t channel) {
    while (true) {
        while (mmio_read(MAIL_STATUS) & MAIL_EMPTY) {}
        uint32_t data = mmio_read(BOARD_MAILBOX_BASE);
        uint32_t read_chan = data & 0xF;
        data >>= 4;
        if (read_chan == channel) {
            return data;
        }
    }
}

void bcm_mailbox_write(uint8_t channel, uint32_t data) {
    while (mmio_read(MAIL_STATUS) & MAIL_FULL) {}
    mmio_write(MAIL_WRITE, data + channel);
}
