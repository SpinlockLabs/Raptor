#include "../hex.h"

void puthex(int value) {
    char buf[256];
    itoa(value, buf, 16);

    puts("0x");
    puts(buf);
}
