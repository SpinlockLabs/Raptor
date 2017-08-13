#include "../common.h"
#include "../abort.h"

does_not_return void __stack_chk_fail(void) {
    abort("Stack check failed.");
}

does_not_return void __stack_chk_fail_local(void) {
    __stack_chk_fail();
}
