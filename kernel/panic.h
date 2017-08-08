#pragma once

#include <liblox/common.h>

/*
 * Call a kernel panic.
 */
noreturn void panic(nullable char* msg);
