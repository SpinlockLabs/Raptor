#include <liblox/common.h>
#include <liblox/io.h>

// Architecture-specific panic handler.
extern void (*arch_panic_handler)(nullable char* msg);

/*
 * Call a kernel panic.
 */
noreturn void panic(nullable char *msg);
