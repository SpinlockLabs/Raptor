#include "cpuid.h"

#include <cpuid.h>

uint cpuid_get_processor_type(void) {
#if !defined(__COMPCERT__)
    uint eax = 0, ebx = 0, ecx = 0, edx = 0;
    __get_cpuid(0, &eax, &ebx, &ecx, &edx);
    return ebx;
#else
    return 0;
#endif
}
