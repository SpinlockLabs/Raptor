#include "userspace.h"

void userspace_jump(uintptr_t location, uintptr_t stack) {
    asm volatile("cli");

    enter_userspace(location, stack);
}
