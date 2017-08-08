#pragma once

#ifdef __GNUC__
#define weak_alias(old, name) \
	extern __typeof(old) name __attribute__((weak, alias(#old)))
#else
#define weak_alias(old, name)
#endif
