#include <liblox/common.h>
#include <liblox/io.h>
#include <liblox/lox-internal.h>

#include <kernel/entry.h>
#include <kernel/cpu/task.h>
#include <kernel/spin.h>

#include "gpio.h"
#include "uart.h"
#include "fb.h"

void lox_output_string_uart(char *str) {
    uart_puts(str);
}

void lox_output_char_uart(char c) {
    uart_putc((unsigned char) c);
}

ulong timer_get_ticks(void) {
    return 0;
}

void spin_lock(spin_lock_t lock) {}
void spin_unlock(spin_lock_t lock) {}

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

void kernel_modules_load(void) {}
void paging_init(void) {}

char* (*arch_get_cmdline)(void) = arch_arm_rpi_get_cmdline;
void (*lox_output_string_provider)(char*) = lox_output_string_uart;
void (*lox_output_char_provider)(char) = lox_output_char_uart;

void rpi_tick(void) {
    static bool state = false;
    static bool blink_act = true;
    static unsigned int counter = 0;

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

    cpu_task_queue(rpi_tick);
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

    framebuffer_init(640, 480);
    puts(DEBUG "Framebuffer initialized.\n");

    cpu_task_queue(rpi_tick);

    kernel_init();
}

void cpu_run_idle(void) {
    while (true) {
        cpu_task_queue_flush();
    }
}
