/**
 * Implements atomics when not supported by the compiler.
 */
#pragma once

#if defined(__STDC_NO_ATOMICS__) || \
  defined(__EMSCRIPTEN__) || \
  __STDC_VERSION__ < 201112L

#ifdef __GNUC__
#define atomic_fetch_add __sync_add_and_fetch
#define atomic_fetch_sub __sync_sub_and_fetch
#define atomic_exchange __sync_lock_test_and_set
#define atomic_store __sync_lock_test_and_set
#else
#define atomic_fetch_add(a, b) (*(a) = *(a) + (b))
#define atomic_fetch_sub(a, b) (*(a) = *(a) - (b))
#define atomic_exchange(a, b) (*(a))
#define atomic_store(a, b) (*(a) = (b))
#endif

#define atomic_int32 volatile int32_t

#else
#include <stdatomic.h>
#define atomic_int32 _Atomic int32_t
#endif
