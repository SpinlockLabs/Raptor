#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "entry.h"
#include "idt.h"
#include "tty.h"

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

void reverse(char* str, int len) {
    int start = 0;
    int end = len - 1;
    while (start < end) {
        char tmp = str[start];
        str[start] = str[end];
        str[end] = tmp;
        start++;
        end--;
    }
}

char* itoa(int num, char* str, int base) {
    int i = 0;
    bool neg = false;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (num < 0) {
        neg = true;
        num = -num;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - base) + 'a' : rem + '0';
        num = num / base;
    }

    if (neg) {
        str[i++] = '-';
    }

    str[i] = '\0';

    reverse(str, i);

    return str;
}

void kernel_main(void) {
    gdt_init();
    idt_init();
    isr_init();
    terminal_init();

    asm volatile("int $0x0");

    terminal_writestring("Hello World!\n");
    terminal_writestring("Hello World!\n");
}
