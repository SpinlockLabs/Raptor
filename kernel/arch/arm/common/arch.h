#pragma once

#if defined(ARCH_ARM_RPI)
#include "../rpi/arch.h"
#else
#error Unknown ARM target.
#endif
