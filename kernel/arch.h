/* Architecture-specific headers. */
#pragma once

#ifdef ARCH_X86
#include <kernel/arch/x86/arch.h>
#else
#error Unknown architecture.
#endif
