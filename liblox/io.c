#include "lox-internal.h"

void puts(char *msg) {
    if (lox_output_string_provider != NULL) {
        lox_output_string_provider(msg);
    }
}
