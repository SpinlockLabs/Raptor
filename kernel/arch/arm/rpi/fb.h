#pragma once

#include <liblox/graphics/color.h>
#include <stdint.h>

void framebuffer_get_size(uint32_t*, uint32_t*);
void framebuffer_init(uint32_t, uint32_t);
void framebuffer_clear(uint32_t);
void framebuffer_set(
    uint32_t x,
    uint32_t y,
    uint32_t color
);
