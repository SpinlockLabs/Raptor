#include "pen.h"

bool gpen_set_pixel(gpen_t* pen, uint32_t x, uint32_t y, rgb_t* rgb) {
    if (pen == NULL || rgb == NULL) {
        return false;
    }

    if (x >= pen->width ||
        y >= pen->height) {
        return false;
    }

    if (pen->ops.set_pixel == NULL) {
        return false;
    }

    return pen->ops.set_pixel(pen, x, y, rgb);
}
