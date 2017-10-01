/* Architecture-specific headers. */
#pragma once

#if defined(ARCH_X86)
#include "arch/x86/arch.h"
#elif defined(ARCH_ARM)
#include "arch/arm/common/arch.h"
#elif defined(ARCH_USER)
#include "arch/user/arch.h"
#elif defined(ARCH_NONE)
#include "arch/generic/nommu.h"
#include "arch/generic/nosched.h"
#include "arch/generic/nouser.h"
#else
#error Unknown architecture.
#endif
