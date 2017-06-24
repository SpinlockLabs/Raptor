#pragma once

#include <stddef.h>

// Specifies that a struct should be packed.
#define packed __attribute__((packed))

// Specifies that an entity is used.
#define used __attribute__((used))

// Specifies that an entity is deprecated.
#define deprecated __attribute__((deprecated))

// Specifies that a function never returns.
#define noreturn __attribute__((noreturn))

// Specifies that a function never modifies its arguments.
#define constf __attribute__((const))

// Specifies that a given function argument is unused.
#define unused(name) (void) name

// Specifies that the value a is likely to be the same as value b.
#define likely_val(a, b) __builtin_expect((a), (b))

// Specifies that x is likely to be true.
#define likely(x) likely_val(x, 1)

// Specifies that x is unlikely to be true.
#define unlikely(x) likely_val(x, 0)

// Specifies that a function should always be inlined.
#define always_inline __attribute__((always_inline))

// Specifies that a function will be called often.
#define hot __attribute__((hot))

// Specifies that a function will not be called often.
#define cold __attribute__((cold))

// Specifies that the location in the program is unreachable.
#define unreachable() __builtin_unreachable()

// Allocate the amount given by size on the stack.
inline void* stack_allocate(size_t size) {
    return __builtin_alloc(size);
}

// Allocate the amount given by size and aligned by alignment on the stack.
inline void* stack_allocate_aligned(size_t size, size_t alignment) {
    return __builtin_alloca_with_align(size, alignment);
}
