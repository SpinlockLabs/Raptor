#include <liblox/common.h>
#include <liblox/hex.h>
#include <liblox/memory.h>

#include "fb.h"
#include "mailbox.h"
#include "board.h"

typedef struct fb_mbox_init {
    uint32_t w;
    uint32_t h;
    uint32_t vw;
    uint32_t vh;
    uint32_t pitch;
    uint32_t depth;
    uint32_t x;
    uint32_t y;
    uint32_t buffer;
    uint32_t size;
} fb_mbox_init_t;

static fb_mbox_init_t* pi_fbinfo;
static uint32_t* pi_framebuffer;

static uint32_t fb_offset(uint32_t x, uint32_t y) {
    uint32_t pitch = pi_fbinfo->pitch / sizeof(uint32_t);
    return ((y * pitch) + x);
}

void framebuffer_init(uint32_t w, uint32_t h) {
    uintptr_t ptr = (uintptr_t) zalloc(0x1000 + sizeof(fb_mbox_init_t));
    ptr &= 0xFFFFF000;
    ptr += 0x1000;

    pi_fbinfo = (fb_mbox_init_t*) ptr;

    pi_fbinfo->w = w;
    pi_fbinfo->h = h;
    pi_fbinfo->vw = w;
    pi_fbinfo->vh = h;
    pi_fbinfo->pitch = 0;
    pi_fbinfo->depth = 32;
    pi_fbinfo->x = 0;
    pi_fbinfo->y = 0;
    pi_fbinfo->buffer = 0;
    pi_fbinfo->size = 0;

    uint32_t mbox_address = BOARD_BUS_ADDRESS(ptr);
    bcm_mailbox_write(1, mbox_address);
    uint32_t result = bcm_mailbox_read(1);

    if (result != 0 || pi_fbinfo->buffer == 0 || pi_fbinfo->depth != 32) {
        printf(ERROR "Failed to initialize framebuffer.\n");
        return;
    }

    pi_framebuffer = (uint32_t*) (pi_fbinfo->buffer & 0x3FFFFFFF);

    printf(
        DEBUG "Framebuffer Size: %d x %d\n",
        pi_fbinfo->w,
        pi_fbinfo->h
    );

    framebuffer_clear(rgba32_white.color);

    extern void draw_spinlock_logo(void);
    draw_spinlock_logo();
}

void framebuffer_clear(uint32_t color) {
    uint32_t w = pi_fbinfo->w;
    uint32_t h = pi_fbinfo->h;

    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            uint32_t offset = fb_offset(x, y);

            pi_framebuffer[offset] = color;
        }
    }
}

void framebuffer_set(
    uint32_t x,
    uint32_t y,
    uint32_t color) {
    uint32_t offset = fb_offset(x, y);
    pi_framebuffer[offset] = color;
}

void framebuffer_get_size(uint32_t* w, uint32_t* h) {
    *w = pi_fbinfo->w;
    *h = pi_fbinfo->h;
}
