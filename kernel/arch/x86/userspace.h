#pragma once

#include <stdint.h>

void userspace_jump(void* location, uintptr_t stack);

extern void enter_userspace(void* location, uintptr_t stack);
