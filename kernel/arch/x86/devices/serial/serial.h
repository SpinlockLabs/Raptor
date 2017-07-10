#pragma once

#include <liblox/common.h>

#include <kernel/tty.h>
#include <kernel/cpu/task.h>

typedef struct tty_serial {
    tty_t* tty;
    uint16_t port;
    task_id poll_task;
    bool echo;
} tty_serial_t;

tty_serial_t* tty_create_serial(char* name, uint index);