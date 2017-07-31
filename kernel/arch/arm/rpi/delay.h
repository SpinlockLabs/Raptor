#pragma once

#include <liblox/common.h>

#include <stdint.h>

/* Loop <count> times in a way that the compiler won't optimize away. */
void delay(ulong count);
