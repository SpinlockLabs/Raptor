#pragma once

#define __LIBLOX_COMMON_DEF

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef UINT_MAX
#include <limits.h>
#endif

#if defined(RAPTOR_KERNEL) && !defined(__KERNEL_COMMON_DEF)
#include <kernel/common.h>
#endif

#if UINTPTR_MAX <= UINT_MAX
// Machine is 32-bit.
#define BITS_32
#else
// Machine is 64-bit.
#define BITS_64
#endif

#if defined(_MSC_VER) && !defined(__clang__)
// Specifies that a struct should be packed.
#define packed __declspec(align(1))

// Disable attributes.
#define __attribute__(x)

// Disable restrict.
#define restrict

// Variable-length arrays.
#define vla(type, name) type name[1]

// Declares that a function is pure, and has no side effects.
#define pure __attribute__((pure))

// Declares that a function should have no enter and exit instructions.
#define naked _declspec(naked)

// Declares that a function will not return.
#define does_not_return _declspec(noreturn)
#else
// Declares that a struct should be packed.
#define packed __attribute__((packed))

// Variable-length arrays.
#define vla(type, name) type name[]

// Declares that a function is pure, and has no side effects.
#define pure __attribute__((pure))

// Declares that a function should have no enter and exit instructions.
#define naked __attribute__((naked))

// Declares that a function will not return.
#define does_not_return __attribute__((noreturn))
#endif

// Declares that an entity is used.
#define used __attribute__((used))

// Declares that a given function argument is unused.
#define unused(name) (void) name

// Declares that the value a is likely to be the same as value b.
#define likely_val(a, b) __builtin_expect((a), (b))

// Declares that x is likely to be true.
#define likely(x) likely_val(x, 1)

// Declares that x is unlikely to be true.
#define unlikely(x) likely_val(x, 0)

// Declares that a function should always be inlined.
#define always_inline __attribute__((always_inline))

// Declares that the location in the program is unreachable.
#define unreachable() __builtin_unreachable()

// Checks if x is null, and if it is, returns null.
#define ensure_allocated(x) if ((x) == NULL) return NULL

// Specifies that a value can be null.
#define nullable

// Simplified type for unsigned int.
typedef unsigned int uint;

// Simplified type for unsigned long.
typedef unsigned long ulong;

// Simplified type for unsigned short.
typedef unsigned short ushort;

// Simplified type for unsigned char.
typedef unsigned char uchar;
