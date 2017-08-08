#include "color.h"

uint32_t rgb_as(rgb_t* input, pixel_fmt_t fmt) {
    if (fmt == PIXEL_FMT_RGBA32) {
        rgba32_t pix = {
            .r = input->r,
            .g = input->g,
            .b = input->b,
            .a = 255
        };

        return pix.color;
    }
    return 0;
}
