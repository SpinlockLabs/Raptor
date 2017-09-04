#include "loxcall.h"
#include "table.h"

#include <kernel/process/process.h>

syscall_result_t loxcall_exit(int code) {
    unused(code);
    process_t* process = process_get_current();
    process->status = PROCESS_WAITING;
    return 0;
}

syscall_result_t loxcall_console_write(uint8_t* bytes, size_t size) {
    process_t* process = process_get_current();
    if (process->tty != NULL) {
        tty_write(process->tty, bytes, size);
        return 0;
    }
    return 1;
}

void syscall_loxcall_init(void) {
    syscall_add(0, SYSCALL_EXIT, loxcall_exit);
    syscall_add(0, SYSCALL_CONSOLE_WRITE, loxcall_console_write);
}
