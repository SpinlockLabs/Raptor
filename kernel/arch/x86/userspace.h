#pragma once

#include <stdint.h>

void userspace_jump(uintptr_t location, uintptr_t stack);

extern void enter_userspace(uintptr_t location, uintptr_t stack);
