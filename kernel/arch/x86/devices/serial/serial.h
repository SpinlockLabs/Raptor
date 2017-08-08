#pragma once

#include <liblox/common.h>

#include <kernel/tty.h>
#include <kernel/cpu/task.h>

typedef struct tty_serial {
    tty_t* tty;
    uint16_t port;
    ktask_id poll_task;
} tty_serial_t;

tty_serial_t* tty_create_serial(char* name, uint index);
