#include "lox-internal.h"

void puts(char *msg) {
    if (lox_output_string_provider != NULL) {
        lox_output_string_provider(msg);
    }
}

void putc(char c) {
    if (lox_output_char_provider != NULL) {
        lox_output_char_provider(c);
    }
}
