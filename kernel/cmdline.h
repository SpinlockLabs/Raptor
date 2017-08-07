#pragma once

#include <stdbool.h>
#include <stdint.h>

bool cmdline_bool_flag(char *name);
char* cmdline_get(void);
char* cmdline_read(char* name);
