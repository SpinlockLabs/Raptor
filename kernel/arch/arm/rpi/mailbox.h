#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <liblox/common.h>

#include "mmio.h"
#include "board.h"

uint32_t bcm_mailbox_read(uint8_t channel);
void bcm_mailbox_write(uint8_t channel, uint32_t data);
