#include <liblox/common.h>
#include <stdint.h>

#include "mmio.h"
#include "board.h"

ulong ticks = 0;

void timer_init(uint32_t freq) {
    unused(freq);

    mmio_write(BOARD_TIMER_CTL, 0x00F90000);
    mmio_write(BOARD_TIMER_CTL, 0x00F90200);
}

ulong timer_get_ticks(void) {
    return ticks;
}
