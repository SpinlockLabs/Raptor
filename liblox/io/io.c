#include "../lox-internal.h"
#include "../printf.h"

void puts(char* msg) {
    lox_output_string_provider(msg);
}

void putc(char c) {
    lox_output_char_provider(c);
}

void printf(char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[2048];
    vasprintf(buf, fmt, args);
    va_end(args);
    puts(buf);
}
