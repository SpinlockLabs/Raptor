#include "panic.h"

void (*lox_abort_provider)(char* msg) = panic;
