#include "../string.h"

char* strcpy(char* restrict dest, const char* restrict src) {
    const unsigned char* s = (const unsigned char*) src;
    unsigned char* d = (unsigned char*) dest;
    while ((*d++ = *s++)) {}
    return dest;
}
