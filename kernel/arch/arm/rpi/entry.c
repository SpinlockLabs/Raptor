#include <liblox/common.h>
#include <liblox/lox-internal.h>

#include <liblox/io.h>
#include <liblox/string.h>

#include <kernel/process/process.h>

#include <kernel/entry.h>
#include <kernel/timer.h>
#include <kernel/cpu/task.h>

#include "irq.h"
#include "gpio.h"
#include "uart.h"
#include "fb.h"
#include "delay.h"

#include "usb/usb.h"

void lox_output_string_uart(char *str) {
    uart_puts(str);
}

void lox_output_char_uart(char c) {
    uart_putc((uint8_t) c);
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

syscall_result_t lox_syscall(syscall_id_t id, uintptr_t* args) {
    unused(id);
    unused(args);
    return 0;
}

syscall_result_t (*lox_syscall_provider)(
  syscall_id_t,
  uintptr_t*
) = lox_syscall;

static void uart_tty_write(tty_t* tty, const uint8_t* buf, size_t size) {
    unused(tty);

    uart_write(buf, size);
}

static char uart_conv(char c) {
    if (c == '\r') {
        return '\n';
    }

    if (c == 0x7F) {
        return '\b';
    }
    return c;
}

static void uart_poll_read_task(void* extra) {
    unused(extra);

    if (uart_poll()) {
        uint8_t c = uart_poll_getc();

        if (!uart_tty->flags.raw && c == 0x11) {
            uart_kpload();
            return;
        }

        if (!uart_tty->flags.raw) {
            c = (uint8_t) uart_conv((char) c);
        }

        tty_read_event_t event = {
            .tty = uart_tty,
            .size = 1,
            .data = &c
        };

        epipe_deliver(&uart_tty->reads, &event);
    }
}

void kernel_setup_devices(void) {
    uart_tty = tty_create("uart");
    uart_tty->ops.write = uart_tty_write;
    uart_tty->flags.allow_debug_console = true;
    uart_tty->flags.write_kernel_log = true;
    uart_tty->flags.echo = true;
    tty_register(uart_tty);

    ktask_repeat(1, uart_poll_read_task, NULL);
    framebuffer_init(640, 480);

    usb_rpi_init();
}

void* atags = NULL;

used does_not_return void kernel_main(
  uint32_t r0,
  uint32_t r1,
  uint32_t atags_addr
) {
    (void) r0;
    (void) r1;
    atags = (void*) atags_addr;

    extern uint8_t __bss_start;
    extern uint8_t __end;
    memset(&__bss_start, 0, &__end - &__bss_start);

    uart_init();
    printf("\n");
    printf(DEBUG "UART initialized.\n");

    gpio_init();
    printf(DEBUG "GPIO initialized.\n");

    irq_init();
    printf(DEBUG "IRQ initialized.\n");

    timer_init(1000);
    printf(DEBUG "Timer initialized.\n");

    kernel_init();
}

void arch_process_init_kidle(process_t* process) {
    unused(process);
}

void cpu_run_idle(void) {
    while (true) {
        delay(50000);
        ktask_queue_flush();
    }
}
