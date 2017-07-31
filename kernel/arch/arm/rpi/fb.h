#pragma once

#include <stdint.h>

#define FB_PIXEL(red, green, blue, alpha)  (((red) & 0xFF) \
                    | ((green) & 0xFF) << 8  \
                    | ((blue) & 0xFF) << 16  \
                    | ((alpha) & 0xFF) << 24)

void framebuffer_get_size(uint32_t*, uint32_t*);

void framebuffer_init(uint32_t, uint32_t);

void framebuffer_clear(uint8_t, uint8_t, uint8_t, uint8_t);
void framebuffer_set(
    uint32_t x,
    uint32_t y,
    uint8_t r,
    uint8_t g,
    uint8_t b,
    uint8_t a
);
