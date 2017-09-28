#include "arch.h"
#include "userspace.h"
#include "irq.h"

#include <kernel/process/process.h>

#define PUSH(stack, type, item) stack -= sizeof(type); \
                            *((type *) (stack)) = item

void enter_user_jmp(uintptr_t location, int argc, char** argv, uintptr_t stack) {
    int_disable();

    set_kernel_stack(process_get_current()->image.stack);

    PUSH(stack, uintptr_t, (uintptr_t) argv);
    PUSH(stack, int, argc);
    do_enter_userspace(location, stack);
}
