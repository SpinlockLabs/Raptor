/**
 * Implements atomics when not supported by the compiler.
 */
#pragma once

#if defined(__STDC_NO_ATOMICS__) || __STDC_VERSION__ < 201112L
#warning We are faking atomics. Do not use this library seriously, it is very unstable.
#define atomic_fetch_add(a, b) (*(a) = *(a) + (b))
#define atomic_fetch_sub(a, b) (*(a) = *(a) - (b))
#define atomic_exchange(a, b) \
  __extension__({ \
    unsigned int ___tmp = *(a); *(a) = (b); ___tmp; \
  })
#define atomic_store(a, b) (*(a) = (b))
#else
#include <stdatomic.h>
#endif
