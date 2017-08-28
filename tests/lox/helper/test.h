#pragma once

#include <stdbool.h>

char* test_current_id();
void test_start(char* id);
void test_end(void);
void test_fail(void);
void test_assert(bool result, char* fail_msg);
