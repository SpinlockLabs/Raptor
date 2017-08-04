/**
 * Graphics color related utilities.
 */
#pragma once

#include <stdint.h>

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
} rgba32;

/**
 * White in RGBA32.
 */
extern rgba32 rgba32_white;
