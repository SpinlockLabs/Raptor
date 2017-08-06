/**
 * Graphics color related utilities.
 */
#pragma once

#include "../common.h"

#include <stdint.h>

/**
 * Pixel formats.
 */
typedef enum pixel_fmt {
    PIXEL_FMT_RGBA32
} pixel_fmt_t;

/**
 * RGB + Alpha color structure.
 */
typedef struct rgba32 {
    union {
        struct {
            uint8_t r : 8;
            uint8_t g : 8;
            uint8_t b : 8;
            uint8_t a : 8;
        };

        uint32_t color;
    };
} packed rgba32_t;

typedef struct rgb {
    union {
        struct {
            uint8_t r : 8;
            uint8_t g : 8;
            uint8_t b : 8;
        };

        uint32_t color;
    };
} packed rgb_t;

uint32_t rgb_as(rgb_t* pixel, pixel_fmt_t fmt);
