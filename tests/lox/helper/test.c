#include "test.h"

#include <stddef.h>

#include <liblox/io.h>
#include <liblox/abort.h>

char* __current_test = NULL;
char* __last_test = NULL;

char* test_current_id() {
    return __current_test;
}

void test_start(char* id) {
    if (__current_test != NULL) {
        test_end();
    }
    __current_test = id;
    printf("[Begin] %s\n", __current_test);
}

void test_end(void) {
    printf("[End] %s\n", __current_test);
    __last_test = __current_test;
    __current_test = NULL;
}

void test_fail(void) {
    printf("[Fail] %s\n", __current_test);
    __last_test = __current_test;
    __current_test = NULL;
    abort(NULL);
}

void test_assert(bool result, char* fail_msg) {
    if (result) return;
    printf("[Assertion Failed] %s\n", fail_msg);
    test_fail();
}
