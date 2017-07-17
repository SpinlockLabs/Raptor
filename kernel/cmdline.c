#include <liblox/common.h>
#include <liblox/string.h>
#include <liblox/printf.h>
#include <liblox/memory.h>
#include <liblox/io.h>

#include "cmdline.h"

extern char* (*arch_get_cmdline)(void);

char* cmdline_get(void) {
    if (arch_get_cmdline != NULL) {
        return arch_get_cmdline();
    }

    return "";
}

#define MAX_FLAG_SIZE 256

bool cmdline_bool_flag(char *name) {
    char *ptr = cmdline_get();

    if (ptr == NULL) {
        return false;
    }

    char buf[MAX_FLAG_SIZE];
    memset(buf, 0, MAX_FLAG_SIZE);

    uint i = 0;

    while (*ptr != '\0') {
        if (*ptr == ' ') {
            if (strcmp(buf, name) == 0) {
                return true;
            }
            i = 0;
            memset(buf, 0, MAX_FLAG_SIZE);
            ptr++;
            continue;
        }

        if (i >= MAX_FLAG_SIZE) {
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

char* cmdline_read(char* name) {
    size_t size = strlen(name);

    char name_with_equals[256] = "";
    sprintf(name_with_equals, "%s=", name);

    char* cmdline = strdup(cmdline_get());
    char* state = cmdline;

    char* token;
    while ((token = strtok(state, " ", &state)) != NULL) {
        char* result = strstr(token, name_with_equals);
        if (result == token) {
            free(cmdline);
            return strdup(token + size + 1);
        }
    }

    free(cmdline);
    return NULL;
}
