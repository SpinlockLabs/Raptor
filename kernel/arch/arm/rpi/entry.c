#include <liblox/common.h>
#include <liblox/io.h>
#include <liblox/lox-internal.h>
#include <kernel/timer.h>
#include <stdbool.h>
#include <stdint.h>

#include "delay.h"
#include "gpio.h"
#include "uart.h"
#include "fb.h"

void lox_output_string_uart(char *str) {
    uart_puts(str);
}

void lox_output_char_uart(char c) {
    uart_putc((unsigned char) c);
}

used void arch_panic_handler(char *str) {
    lox_output_string_uart("[PANIC]");

    if (str != NULL) {
        lox_output_string_uart(str);
    }

    lox_output_char_uart('\n');
}

char* arch_arm_rpi_get_cmdline(void) {
    return "";
}

char* (*arch_get_cmdline)(void) = arch_arm_rpi_get_cmdline;
void (*lox_output_string_provider)(char*) = lox_output_string_uart;
void (*lox_output_char_provider)(char) = lox_output_char_uart;

used noreturn void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();
    puts("\n");
    puts(INFO "Raptor kernel\n");
    puts(DEBUG "UART initialized.\n");

    gpio_init();
    puts(DEBUG "GPIO initialized.\n");

//    timer_init(250);
//    puts(DEBUG "Timer initialized.\n");

    framebuffer_init(640, 480);
    puts(DEBUG "Framebuffer initialized.\n");

    bool state = false;
    bool blink_act = true;
    unsigned int counter = 0;

    while (true) {
        if (uart_poll()) {
            unsigned char c = uart_poll_getc();
            char cc = (char) c;

            if (cc == 't') {
                blink_act = !blink_act;
            }

            uart_putc(c);
        }

        counter++;

        if (counter == 50000) {
            counter = 0;
            state = !state;

            if (blink_act) {
                gpio_set_act_led_state(state);
            }
        }
    }
}
