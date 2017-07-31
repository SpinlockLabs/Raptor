#include <liblox/common.h>
#include <liblox/io.h>
#include <liblox/sleep.h>
#include <liblox/lox-internal.h>

#include <kernel/entry.h>
#include <kernel/tty.h>
#include <kernel/timer.h>

#include <kernel/cpu/task.h>
#include <liblox/string.h>

#include "gpio.h"
#include "uart.h"
#include "fb.h"
#include "delay.h"

void lox_output_string_uart(char *str) {
    uart_puts(str);
}

void lox_output_char_uart(char c) {
    uart_putc((unsigned char) c);
}

used void arch_panic_handler(char *str) {
    lox_output_string_uart("[PANIC] ");

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
void (*lox_sleep_provider)(ulong) = delay;

static tty_t* uart_tty = NULL;

static void uart_tty_write(tty_t* tty, const uint8_t* buf, size_t size) {
    unused(tty);

    uart_write(buf, size);
}

static void uart_poll_read_task(void* extra) {
    unused(extra);

    if (uart_poll()) {
        unsigned char c = uart_poll_getc();

        if (c == '\r') {
            c = '\n';
            uart_putc('\r');
        }

        if (c == 127) {
            c = '\b';
        }

        uart_putc(c);

        if (uart_tty->handle_read != NULL) {
            uart_tty->handle_read(uart_tty, &c, 1);
        }
    }
}

void kernel_setup_devices(void) {
    uart_tty = tty_create("uart");
    uart_tty->write = uart_tty_write;
    uart_tty->flags.allow_debug_console = true;
    uart_tty->flags.write_kernel_log = true;
    tty_register(uart_tty);

    ktask_repeat(1, uart_poll_read_task, NULL);

    framebuffer_init(640, 480);
}

used noreturn void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void) r0;
    (void) r1;
    (void) atags;

    extern unsigned char __bss_start;
    extern unsigned char __end;
    memset(&__bss_start, 0, &__end - &__bss_start);

    uart_init();
    puts("\n");
    puts(DEBUG "UART initialized.\n");

    gpio_init();
    puts(DEBUG "GPIO initialized.\n");

    timer_init(1000);
    puts(DEBUG "Timer initialized.\n");

    kernel_init();
}

extern ulong ticks;

void cpu_run_idle(void) {
    while (true) {
        ticks++;
        ktask_queue_flush();
        sleep(100000);
    }
}
