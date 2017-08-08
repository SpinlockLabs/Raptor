/**
 * Graphics Drawing Pen.
 */
#pragma once

#include <stdint.h>
#include "color.h"

typedef struct gpen gpen_t;

/**
 * A provided function on a pen to set the given pixel
 * to the given RGB value on the backing drawing surface.
 */
typedef bool (*gpen_set_pixel_t)(
    gpen_t* pen,
    uint32_t x,
    uint32_t y,
    rgb_t* rgb
);

/**
 * A Graphics Drawing Pen.
 */
struct gpen {
    /**
     * Backing drawing surface
     * core operations.
     */
    struct {
        /**
         * Sets a pixel on the drawing surface.
         */
        gpen_set_pixel_t set_pixel;
    } ops;

    /**
     * Width of the drawing surface.
     */
    uint32_t width;

    /**
     * Height of the drawing surface.
     */
    uint32_t height;
};

/**
 * Sets the given pixel on the drawing surface to the
 * given RGB value.
 * @param pen graphics pen
 * @param x pixel x
 * @param y pixel y
 * @param rgb rgb value
 */
bool gpen_set_pixel(gpen_t* pen, uint32_t x, uint32_t y, rgb_t* rgb);
