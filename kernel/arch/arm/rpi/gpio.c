#include <stdint.h>

#include "gpio.h"
#include "kernel/arch/arm/common/mmio.h"

#define GPIO_ACT_LED_SEL_BIT 21
#define GPIO_ACT_LED_BIT 15

void gpio_init(void) {
    /* Enable ACT LED */
    mmio_op(GPIO_SEL4) |= 1 << GPIO_ACT_LED_SEL_BIT;
}

void gpio_set_act_led_state(bool state) {
#ifdef PI_1
    state = !state;
#endif

    mmio_op(state ? GPIO_SET1 : GPIO_CLR1) |= 1 << GPIO_ACT_LED_BIT;
}
