#include <liblox/syscall.h>

#ifndef ARCH_USER
syscall_result_t __syscall(syscall_id_t id, uintptr_t* args) {
    syscall_result_t result = 0;

    asm volatile(
        "movl %1, %%eax\n"
        "movl %2, %%ebx\n"
        "int $0x80\n"
        "movl %%eax, %0"
        : "=r"(result)
        : "r"(id), "r"(args)
        : "eax", "ebx"
    );

    return result;
}
#else
syscall_result_t __syscall(syscall_id_t id, uintptr_t* args) {
    syscall_result_t result = 0;

    asm volatile(
        "movl %1, %%eax\n"
        "movl %2, %%ebx\n"
        "int $0x80\n"
        "movl %%eax, %0"
        : "=r"(result)
        : "r"(id), "r"(args[0])
        : "eax", "ebx"
    );
    return result;
}
#endif
