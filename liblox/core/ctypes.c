#include "../ctypes.h"

bool isspace(int c) {
    return c == ' ' || (unsigned) c - '\t' < 5;
}

bool isdigit(int c) {
    return (unsigned) c - '0' < 10;
}

bool isalpha(int c) {
    return ((unsigned) c | 32) - 'a' < 26;
}

bool isupper(int c) {
    return (unsigned) c - 'A' < 26;
}

bool islower(int c) {
    return (unsigned) c - 'a' < 26;
}
