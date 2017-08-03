#include "../string.h"
#include "../memory.h"

bool strdelcat(char* buf, unsigned int idx) {
    size_t len = strlen(buf);

    if (idx >= len) {
        return false;
    }

    if (idx == 0) {
        memcpy(buf, buf + 1, len - 1);
        buf[len - 1] = '\0';
        return true;
    }

    char* tmp = calloc(1, idx);
    memcpy(tmp, buf, idx);
    memcpy(buf + idx, buf + idx + 1, len - idx - 1);
    memcpy(buf, tmp, idx);
    buf[len - 1] = '\0';
    free(tmp);
    return true;
}
