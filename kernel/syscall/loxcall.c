#include "loxcall.h"
#include "table.h"

#include <kernel/process/process.h>
#include <kernel/process/scheduler.h>

syscall_result_t loxcall_exit(uintptr_t* args) {
    int code = args[0];
    process_t* process = process_get_current();
    process->status = PROCESS_STOPPED;
    process->exit_code = code;
    scheduler_switch_next();
    return 0;
}

syscall_result_t loxcall_console_write(uintptr_t* args) {
    uint8_t* bytes = (uint8_t*) args[0];
    size_t size = args[1];
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
