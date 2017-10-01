/**
 * kexec loader.
 */
#pragma once

#include <stddef.h>

#include <liblox/common.h>

extern arch_specific void kexec(const void* kernel, size_t size);
