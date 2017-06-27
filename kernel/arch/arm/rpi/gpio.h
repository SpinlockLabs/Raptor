#pragma once

#include <stdbool.h>

enum {
#ifdef PI_1
    GPIO_BASE = 0x20200000,
#else
    GPIO_BASE = 0x3F200000,
#endif
    GPPUD = (GPIO_BASE + 0x94), // Controls actuation of pull up/down to ALL GPIO pins.
    GPPUDCLK0 = (GPIO_BASE + 0x94), // Controls actuation of pull up/down for specific GPIO pin.

#define _GPF(i, sel, set, clr) \
    GPIO_SEL ## i = (GPIO_BASE + (sel)), \
    GPIO_SET ## i = (GPIO_BASE + (set)), \
    GPIO_CLR ## i = (GPIO_BASE + (clr)), \

    _GPF(1, 0x04, 0x20, 0x2C)
#undef _GPF

    GPIO_SEL4 = (GPIO_BASE + 0x10),
};

void gpio_init(void);
void gpio_set_act_led_state(bool);
