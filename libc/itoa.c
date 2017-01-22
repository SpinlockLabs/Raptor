#include "string.h"

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

