/**
 * Framebuffer devices.
 */
#pragma once

#include <stdint.h>

#include <liblox/graphics/color.h>

#include <kernel/device/registry.h>

typedef struct framebuffer framebuffer_t;

typedef void (*framebuffer_destroy_t)(framebuffer_t* fb);

struct framebuffer {
    /**
     * Framebuffer name.
     */
    char name[64];

    /**
     * Pixel format.
     */
    pixel_fmt_t format;

    /**
     * Buffer width.
     */
    uint32_t width;

    /**
     * Buffer height.
     */
    uint32_t height;

    /**
     * Buffer pitch.
     */
    uint32_t pitch;

    /**
     * Frame buffer.
     */
    void* buffer;

    /**
     * Device entry.
     */
    device_entry_t* entry;

    /**
     * Operations.
     */
    struct {
        /**
         * Destroy operation.
         */
        framebuffer_destroy_t destroy;
    } ops;

    /**
     * Internal data.
     */
    struct {
        void* provider;
        void* owner;
    } internal;
};

/**
 * Create a framebuffer.
 * @param name framebuffer name.
 * @return unregistered framebuffer.
 */
framebuffer_t* framebuffer_create(
    char* name
);

/**
 * Register a framebuffer.
 * @param parent parent device.
 * @param fb framebuffer.
 */
void framebuffer_register(
    device_entry_t* parent,
    framebuffer_t* fb
);

/**
 * Destroy a framebuffer.
 * @param fb framebuffer.
 */
void framebuffer_destroy(
    framebuffer_t* fb
);

/**
 * Clear the given framebuffer using the given pixel.
 * @param fb framebuffer.
 * @param pixel pixel spec.
 */
void framebuffer_clear(framebuffer_t* fb, uint32_t pixel);
