#include <liblox/common.h>
#include <liblox/io.h>
#include <liblox/lox-internal.h>
#include <stdbool.h>
#include <stdint.h>

#include "uart.h"

void lox_output_string_uart(char *str) {
    uart_puts(str);
}

void lox_output_char_uart(char c) {
    uart_putc((unsigned char) c);
}

void (*lox_output_string_provider)(char*) = lox_output_string_uart;
void (*lox_output_char_provider)(char) = lox_output_char_uart;

used noreturn void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();
    puts(INFO "Raptor kernel\n");

    while (true) {
        unsigned char c = uart_getc();
        uart_putc(c);
    }
}
