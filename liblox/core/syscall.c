#include "../syscall.h"
#include "../va_list.h"
#include "../lox-internal.h"

syscall_result_t syscall(syscall_id_t id, ...) {
    va_list args;
    va_start(args, id);
    syscall_result_t result = lox_syscall_provider(id, &args);
    va_end(args);
    return result;
}
