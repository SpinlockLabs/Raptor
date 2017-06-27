#include <liblox/common.h>
#include <liblox/string.h>

#include "cmdline.h"

static const uint32_t kMaxArgLength = 256;

char *cmdline = NULL;

void init_cmdline(multiboot_t *mboot) {
    cmdline = (char*) mboot->cmdline;
}

bool cmdline_bool_flag(char *name) {
    char *ptr = cmdline;

    if (ptr == NULL) {
        return false;
    }

    char buf[kMaxArgLength];
    memset(buf, 0, kMaxArgLength);

    int i = 0;

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
