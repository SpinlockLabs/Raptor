#pragma once

#define packed __attribute__((packed))
#define used __attribute__((used))
#define deprecated __attribute__((deprecated))
#define noreturn __attribute__((noreturn))
#define constf __attribute__((const))
#define unused(name) (void) name
#define likely_val(a, b) __builtin_expect((a), (b))
#define likely(x) likely_val(x, 1)
#define unlikely(x) likely_val(x, 0)
