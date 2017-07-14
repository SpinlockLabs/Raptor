#pragma once

#ifndef do_div
#define do_div(n,base) ({ \
	uint32_t __base = (base); \
	uint32_t __rem; \
	__rem = ((uint64_t)(n)) % __base; \
	(n) = ((uint64_t)(n)) / __base; \
	__rem; \
 })
#endif

#ifndef ACPI_DIV_64_BY_32
#define ACPI_DIV_64_BY_32(n_hi, n_lo, d32, q32, r32) \
    do { \
        uint64_t (__n) = ((uint64_t) n_hi) << 32 | (n_lo); \
        (r32) = do_div ((__n), (d32)); \
        (q32) = (uint32_t) (__n); \
    } while (0)
#endif

#ifndef ACPI_SHIFT_RIGHT_64
#define ACPI_SHIFT_RIGHT_64(n_hi, n_lo) \
    do { \
        (n_lo) >>= 1; \
        (n_lo) |= (((n_hi) & 1) << 31); \
        (n_hi) >>= 1; \
    } while(0)
#endif
