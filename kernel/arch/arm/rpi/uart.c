#include <liblox/string.h>
#include <kernel/kexec.h>

#include "gpio.h"
#include "kernel/arch/arm/common/mmio.h"
#include "uart.h"

tty_t* uart_tty = NULL;

void uart_init(void) {
    // Disable UART0.
    mmio_write(UART0_CR, 0x00000000);

    // Setup the GPIO pin 14 && 15.

    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    mmio_write(GPPUD, 0x00000000);

    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));

    // Write 0 to GPPUDCLK0 to make it take effect.
    mmio_write(GPPUDCLK0, 0x00000000);

    // Clear pending interrupts.
    mmio_write(UART0_ICR, 0x7FF);

    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // UART_CLOCK = 3000000; Baud = 115200.

    // Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
    mmio_write(UART0_IBRD, 1);
    mmio_write(UART0_FBRD, 40);

    // Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
    mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    // Mask all interrupts.
    mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
                           (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
    // Enable UART0, receive & transfer part of UART.
    mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_putc(uint8_t byte) {
    // Wait for UART to become ready to transmit.
    while (mmio_read(UART0_FR) & (1 << 5)) {}
    mmio_write(UART0_DR, byte);
}

uint8_t uart_poll_getc(void) {
    return (uint8_t) mmio_read(UART0_DR);
}

bool uart_poll(void) {
    return (mmio_read(UART0_FR) & (1 << 4)) ? false : true;
}

uint8_t uart_getc(void) {
    // Wait for UART to receive something.
    while (true) {
        if (!(mmio_read(UART0_FR) & (1 << 4))) {
            break;
        }
    }
    return (uint8_t) mmio_read(UART0_DR);
}

uint32_t uart_read32(void) {
    uint8_t b = 0;
    uint32_t value = uart_getc();

    b = uart_getc();
    value |= b << 8;
    b = uart_getc();
    value |= b << 16;
    b = uart_getc();
    value |= b << 24;
    return value;
}

void uart_kpload(void) {
    uart_puts("KPL\r\n");
    uint32_t length = uart_read32();

    extern uint32_t kpmalloc_a(uint32_t);
    uint8_t* ptr = (uint8_t*) kpmalloc_a(length);
    for (uint32_t i = 0; i < length; i++) {
        uint8_t b = uart_getc();
        ptr[i] = b;
    }

    kexec(ptr, length);
}

void uart_write(const uint8_t* buffer, size_t size) {
    bool conv = (uart_tty != NULL ? !uart_tty->flags.raw : true);
    for (size_t i = 0; i < size; i++) {
        uint8_t c = buffer[i];
        if (conv) {
            char a = (char) c;
            if (a == '\n') {
                uart_putc('\r');
            }

            if (a == '\b') {
                uart_putc('\b');
                uart_putc(' ');
            }
        }
        uart_putc(c);
    }
}

void uart_puts(const char* str) {
    uart_write((const uint8_t*) str, strlen(str));
}
