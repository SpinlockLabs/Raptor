#include "userspace.h"

void userspace_jump(void* location, uintptr_t stack) {
    asm volatile("cli");

    enter_userspace(location, stack);
}
