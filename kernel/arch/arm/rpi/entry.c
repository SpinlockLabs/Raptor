#include <liblox/common.h>
#include <liblox/io.h>
#include <liblox/lox-internal.h>

#include <kernel/entry.h>
#include <kernel/tty.h>
#include <kernel/timer.h>

#include <kernel/cpu/task.h>

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

static char* rpi_cmdline = "";

char* arch_arm_rpi_get_cmdline(void) {
    return rpi_cmdline;
}

void kernel_modules_load(void) {}
void paging_init(void) {}

char* (*arch_get_cmdline)(void) = arch_arm_rpi_get_cmdline;
void (*lox_output_string_provider)(char*) = lox_output_string_uart;
void (*lox_output_char_provider)(char) = lox_output_char_uart;

static tty_t* uart_tty = NULL;

static void uart_tty_write(tty_t* tty, const uint8_t* buf, size_t size) {
    unused(tty);

    uart_write(buf, size);
}

static void uart_poll_read_task(void* extra) {
    unused(extra);

    if (uart_poll()) {
        unsigned char c = uart_poll_getc();

        if (uart_tty->handle_read != NULL) {
            uart_tty->handle_read(uart_tty, &c, 1);
        }
    }

    ktask_queue(uart_poll_read_task, NULL);
}

void kernel_setup_devices(void) {
    uart_tty = tty_create("uart");
    uart_tty->write = uart_tty_write;
    uart_tty->flags.allow_debug_console = true;
    uart_tty->flags.write_kernel_log = true;
    tty_register(uart_tty);

    ktask_queue(uart_poll_read_task, NULL);

    printf(INFO "Device setup complete.\n");
}

used noreturn void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();
    puts("\n");
    puts(DEBUG "UART initialized.\n");

    gpio_init();
    puts(DEBUG "GPIO initialized.\n");

    timer_init(1000);
    puts(DEBUG "Timer initialized.\n");

    framebuffer_init(640, 480);
    puts(DEBUG "Framebuffer initialized.\n");

    kernel_init();
}

void cpu_run_idle(void) {
    while (true) {
        ktask_queue_flush();
    }
}
