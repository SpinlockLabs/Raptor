/* Architecture-specific headers. */
#pragma once

#if defined(ARCH_X86)
#include "arch/x86/arch.h"
#elif defined(ARCH_ARM)
#include "arch/arm/common/arch.h"
#else
#error Unknown architecture.
#endif
