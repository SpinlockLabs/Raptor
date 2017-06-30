#include "hex.h"

void putint_hex(int value) {
    char buf[256];
    itoa(value, buf, 16);

    puts("0x");
    puts(buf);
}

void putint_phexl(char *prefix, int value) {
    puts(prefix);
    putint_hex(value);
    puts("\n");
}
