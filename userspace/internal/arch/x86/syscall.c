#include <liblox/syscall.h>

syscall_result_t __syscall(syscall_id_t id, uintptr_t* args) {
    syscall_result_t result = 0;

    asm volatile(
        "movl %1, %%eax\n"
        "movl %2, %%ebx\n"
        "int $127\n"
        "movl %%eax, %0"
        : "=r"(result)
        : "r"(id), "r"(args)
        : "eax", "ebx"
    );

    return result;
}
