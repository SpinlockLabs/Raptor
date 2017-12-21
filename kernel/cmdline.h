#pragma once

#include <stdbool.h>

#include <liblox/common.h>

bool cmdline_bool_flag(char *name);
arch_specific char* cmdline_get(void);
char* cmdline_read(char* name);
