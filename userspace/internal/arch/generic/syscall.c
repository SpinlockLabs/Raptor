#include <liblox/common.h>
#include <liblox/syscall.h>

syscall_result_t __syscall(syscall_id_t id, uintptr_t* args) {
    unused(id);
    unused(args);
    return -1;
}
