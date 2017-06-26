#include <liblox/common.h>
#include <liblox/io.h>
#include <liblox/lox-internal.h>
#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "uart.h"

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

void (*lox_output_string_provider)(char*) = lox_output_string_uart;
void (*lox_output_char_provider)(char) = lox_output_char_uart;

used noreturn void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();
    puts(DEBUG "UART initialized.\n");

    gpio_init();
    puts(DEBUG "GPIO initialized.\n");

    puts(INFO "Raptor kernel\n");

    while (true) {
        unsigned char c = uart_getc();
        uart_putc(c);

        if (c == 'o') {
            gpio_set_ok_led_state(true);
        } else if (c == 'p') {
            gpio_set_ok_led_state(false);
        }
    }
}
