#pragma once

#if defined(__GNUC__) && !defined(__APPLE__)
#define weak_alias(old, name) \
	extern __typeof(old) name __attribute__((weak, alias(#old)))
#else
#define weak_alias(old, name)
#endif
