#include "../syscall.h"
#include "../va_list.h"
#include "../lox-internal.h"

syscall_result_t syscall(syscall_id_t id, ...) {
    va_list args;
    va_start(args, id);

    uintptr_t array[] = {
        va_arg(args, uintptr_t),
        va_arg(args, uintptr_t),
        va_arg(args, uintptr_t),
        va_arg(args, uintptr_t),
        va_arg(args, uintptr_t),
        va_arg(args, uintptr_t),
        0
    };
    va_end(args);

    return lox_syscall_provider(id, array);
}
