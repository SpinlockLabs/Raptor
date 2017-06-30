#pragma once

#include <stdint.h>

typedef struct fb_pixel_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} packed fb_pixel_t;

void framebuffer_init(uint32_t, uint32_t);
void framebuffer_clear(fb_pixel_t spec);
