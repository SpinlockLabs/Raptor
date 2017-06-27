#pragma once

#include <stdbool.h>

#include "multiboot.h"

extern char *cmdline;

void init_cmdline(multiboot_t *mboot);
bool cmdline_bool_flag(char *name);
