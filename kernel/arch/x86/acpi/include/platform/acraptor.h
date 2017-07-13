#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define ACPI_USE_SYSTEM_INTTYPES

#define ACPI_MACHINE_WIDTH 32
#define ACPI_SIZE size_t
#define UINT8 uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define UINT64 uint64_t
#define INT8 int8_t
#define INT16 int16_t
#define INT32 int32_t
#define INT64 int64_t
#define BOOLEAN bool

#include <kernel/arch/x86/acpi/raptor/acpi.h>
