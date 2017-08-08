#include "mailbox.h"
#include "mmio.h"
#include "board.h"

#include <stdbool.h>

#define MAIL_FULL 0x80000000
#define MAIL_EMPTY 0x40000000

#define MAIL0_READ (BOARD_MAILBOX_BASE + 0x00)
#define MAIL0_STATUS (BOARD_MAILBOX_BASE + 0x18)

#define MAIL1_WRITE (BOARD_MAILBOX_BASE + 0x20)
#define MAIL1_STATUS (BOARD_MAILBOX_BASE + 0x38)

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
