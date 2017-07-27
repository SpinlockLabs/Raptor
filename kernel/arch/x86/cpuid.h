#pragma once

#include <liblox/common.h>

#define PROCESSOR_TYPE_INTEL 0x756e6547
#define PROCESSOR_TYPE_AMD 0x68747541

uint cpuid_get_processor_type(void);
