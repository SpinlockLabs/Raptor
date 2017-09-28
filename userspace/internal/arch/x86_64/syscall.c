#include <liblox/syscall.h>

#ifndef ARCH_USER
#error x86_64 support for syscalls when not in usermode does not exist.
#else
syscall_result_t __syscall(syscall_id_t id, uintptr_t* args) {
    if (id == 1) id = 60;

    int64_t result = 0;
    uint64_t rid = (uint64_t) id;
    asm volatile(
        "movq %1, %%rax\n"
        "movq %2, %%rdi\n"
        "syscall\n"
        "movq %%rax, %0"
        : "=r"(result)
        : "r"(rid), "r"(args[0])
        : "rax", "rdi"
    );
    return (syscall_result_t) result;
}
#endif
