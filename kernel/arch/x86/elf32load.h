#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool elf32_execute(
    uint8_t* bytes,
    size_t size,
    int argc,
    char** argv,
    int envc,
    char** env);
