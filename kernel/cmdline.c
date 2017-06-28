#include <liblox/common.h>
#include <liblox/string.h>

#include "cmdline.h"

static const uint kMaxArgLength = 256;

extern char* (*arch_get_cmdline)(void);

char* get_cmdline(void) {
    if (arch_get_cmdline != NULL) {
        return arch_get_cmdline();
    }

    return "";
}

bool cmdline_bool_flag(char *name) {
    char *ptr = get_cmdline();

    if (ptr == NULL) {
        return false;
    }

    char buf[kMaxArgLength];
    memset(buf, 0, kMaxArgLength);

    uint i = 0;

    while (*ptr != '\0') {
        if (*ptr == ' ') {
            if (strcmp(buf, name) == 0) {
                return true;
            }
            i = 0;
            memset(buf, 0, kMaxArgLength);
            ptr++;
            continue;
        }

        if (i >= kMaxArgLength) {
            ptr++;
            continue;
        }

        buf[i] = *ptr;
        ptr++;
        i++;
    }

    if (strcmp(buf, name) == 0) {
        return true;
    }

    return false;
}
