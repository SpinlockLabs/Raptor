#pragma once

#include <liblox/common.h>

/*
 * Call a kernel panic.
 */
does_not_return void panic(nullable char* msg);
