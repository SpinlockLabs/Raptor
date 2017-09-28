#pragma once

#include <stdint.h>

extern void do_enter_userspace(uintptr_t location, uintptr_t stack);

void enter_user_jmp(uintptr_t location, int argc, char** argv, uintptr_t stack);
