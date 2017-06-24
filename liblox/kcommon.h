#pragma once

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
