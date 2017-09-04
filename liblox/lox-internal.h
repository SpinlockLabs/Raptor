#pragma once

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "syscall.h"
#include "va_list.h"

extern void (*lox_output_string_provider)(nullable char*);
extern void (*lox_output_char_provider)(char);
extern void* (*lox_allocate_provider)(size_t);
extern void* (*lox_aligned_allocate_provider)(size_t);
extern void* (*lox_reallocate_provider)(void*, size_t);
extern void (*lox_free_provider)(nullable void*);
extern void (*lox_sleep_provider)(ulong ms);
extern void (*lox_abort_provider)(char* msg);
extern syscall_result_t (*lox_syscall_provider)(syscall_id_t id, uintptr_t* args);
