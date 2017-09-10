#include <liblox/common.h>

#include "kernel/arch/arm/common/mmio.h"
#include "board.h"

void timer_init(uint32_t freq) {
    unused(freq);
}

ulong timer_get_ticks(void) {
    ulong a = mmio_read(BOARD_SYS_TIMER_CLO);
    ulong b = mmio_read(BOARD_SYS_TIMER_CHI);
    ulong time = a + b;
    if (time == 0) {
        static ulong ticks = 0;
        return ticks++;
    }

    return time;
}
