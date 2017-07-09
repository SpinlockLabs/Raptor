#include <liblox/common.h>
#include <liblox/io.h>
#include <liblox/lox-internal.h>

/*
 * Call a kernel panic.
 */
noreturn void panic(nullable char *msg);
