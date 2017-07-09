#include <liblox/common.h>
#include <liblox/abort.h>

noreturn void __stack_chk_fail(void) {
    abort("Stack check failed.");
}

noreturn void __stack_chk_fail_local(void) {
    __stack_chk_fail();
}
