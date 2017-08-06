#include "fb.h"

#include <liblox/string.h>

framebuffer_t* framebuffer_create(
    char* name
) {
    framebuffer_t* fb = zalloc(sizeof(framebuffer_t));
    strcpy(fb->name, name);
    return fb;
}

void framebuffer_register(
    device_entry_t* parent,
    framebuffer_t* fb
) {
    fb->entry = device_register(
        parent,
        fb->name,
        DEVICE_CLASS_FRAMEBUFFER,
        fb
    );
}

void framebuffer_destroy(
    framebuffer_t* fb
) {
    if (fb->entry != NULL) {
        device_unregister(fb->entry);
    }

    if (fb->ops.destroy != NULL) {
        fb->ops.destroy(fb);
    }

    free(fb);
}

void framebuffer_clear(framebuffer_t* fb, uint32_t color) {
    uint32_t* buffer = fb->buffer;
    for (uint32_t y = 0; y < fb->height; y++) {
        uint32_t base = y * fb->pitch;

        for (uint32_t x = 0; x < fb->width; x++) {
            buffer[base + x] = color;
        }
    }
}
