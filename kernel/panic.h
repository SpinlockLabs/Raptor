#include <liblox/common.h>
#include <liblox/io.h>

/*
 * Call a kernel panic.
 */
noreturn void panic(nullable char *msg);
