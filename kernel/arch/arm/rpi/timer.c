#include <liblox/common.h>

#include "mmio.h"
#include "board.h"

void arch_timer_init(uint32_t freq) {
    unused(freq);

    mmio_write(BOARD_TIMER_CTL, 0x00F90000);
    mmio_write(BOARD_TIMER_CTL, 0x00F90200);
}

uint32_t arch_timer_get_ticks(void) {
    return mmio_read(BOARD_TIMER_CNT);
}
