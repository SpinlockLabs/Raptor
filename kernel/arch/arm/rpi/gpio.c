#include <stdint.h>

#include "gpio.h"

#define LED_GPFBIT 21
#define LED_GPIO_BIT 15

void gpio_init(void) {
    *((uintptr_t*) GPIO_GPFSEL1) |= (1 << LED_GPFBIT);
}

void gpio_set_ok_led_state(bool state) {
#ifdef PI_1
    state = !state;
#endif

    if (state) {
        *((uintptr_t*) GPIO_GPFSET1) = (1 << LED_GPIO_BIT);
    } else {
        *((uintptr_t*) GPIO_GPFCLR1) = (1 << LED_GPIO_BIT);
    }
}
