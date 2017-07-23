#include "../string.h"

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
