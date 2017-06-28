#include "timer.h"

extern void (*arch_timer_init)(uint32_t);
extern uint32_t (*arch_timer_get_ticks)(void);

void timer_init(uint32_t freq) {
    if (arch_timer_init != NULL) {
        return arch_timer_init(freq);
    }
}

uint32_t timer_get_ticks(void) {
    if (arch_timer_get_ticks != NULL) {
        return arch_timer_get_ticks();
    }
    return 0;
}
