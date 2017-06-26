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

#define _PIN(i, sel, set, clr) \
    GPIO_GPFSEL ## i = (GPIO_BASE + (sel)), \
    GPIO_GPFSET ## i = (GPIO_BASE + (set)), \
    GPIO_GPFCLR ## i = (GPIO_BASE + (clr)), \

    _PIN(1, 0, 8, 11)

#undef _PIN
};

void gpio_init(void);
void gpio_set_ok_led_state(bool);
