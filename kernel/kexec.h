/**
 * kexec loader.
 */
#pragma once

#include <stddef.h>

extern void kexec(const void* kernel, size_t size);
