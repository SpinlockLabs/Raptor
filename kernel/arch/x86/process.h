#pragma once

#include <kernel/tty.h>

void sysexec(
    tty_t* tty,
    char* name,
    uint8_t* bytes,
    size_t size,
    int argc,
    char** argv
);
