#pragma once

#include <stddef.h>

extern void (*lox_output_string_provider)(char*);
extern void (*lox_output_char_provider)(char);
extern void* (*lox_allocate_provider)(size_t);
extern void (*lox_free_provider)(void*);
