#include "../common.h"
#include "../abort.h"

noreturn void __stack_chk_fail(void) {
    abort("Stack check failed.");
}

noreturn void __stack_chk_fail_local(void) {
    __stack_chk_fail();
}
