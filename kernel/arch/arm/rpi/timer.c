#include <liblox/common.h>
#include <stdint.h>

#include "mmio.h"
#include "board.h"
#include "irq.h"

void timer_init(uint32_t freq) {
    irq_unmask(BOARD_IRQ_TIMER3);

    mmio_write(BOARD_SYS_TIMER_CLO, ~(30 * freq));
    mmio_write(
        BOARD_SYS_TIMER_C3,
        mmio_read(BOARD_SYS_TIMER_CLO)
          + freq / 100
    );
}

ulong timer_get_ticks(void) {
    ulong i = mmio_read(BOARD_SYS_TIMER_CHI);
    if (i == 0) {
        static ulong ticks = 0;
        return ticks++;
    }
    return i;
}
