#include <liblox/common.h>
#include <liblox/io.h>
#include <liblox/hex.h>

#include "fb.h"
#include "mailbox.h"
#include "delay.h"

typedef struct fb_mbox_init_t {
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
} packed fb_mbox_init_t;

void *pi_fb_mbox_ptr = (void*) 0x40040000;
static uint8_t *pi_framebuffer;

static uint32_t fb_get_pixel_addr(uint32_t x, uint32_t y) {
    uint32_t offset = ((y * ((fb_mbox_init_t*) pi_fb_mbox_ptr)->pitch) + (x * 3));
    return (uint32_t) (pi_framebuffer + offset);
}

void framebuffer_init(uint32_t w, uint32_t h) {
    mmio_write(0x40040000, w);
    mmio_write(0x40040004, h);
    mmio_write(0x40040008, w);
    mmio_write(0x4004000C, h);
    mmio_write(0x40040010, 0);
    mmio_write(0x40040014, 32);
    mmio_write(0x40040018, 0);
    mmio_write(0x4004001C, 0);
    mmio_write(0x40040020, 0);
    mmio_write(0x40040024, 0);

    bcm_mailbox_write(1, (uint32_t) pi_fb_mbox_ptr);
    delay(5000);
    putint_phexl(DEBUG "Framebuffer Mbox: ", (int) bcm_mailbox_read(1));

    fb_mbox_init_t *init = ((fb_mbox_init_t*) pi_fb_mbox_ptr);

    pi_framebuffer = (uint8_t*) init->buffer;

    framebuffer_clear((fb_pixel_t) {
        .r = 255,
        .g = 0,
        .b = 0
    });

    putint_phexl(DEBUG "Framebuffer Structure: ", (int) pi_fb_mbox_ptr);
    putint_phexl(DEBUG "Framebuffer Location: ", (int) init->buffer);
    putint_phexl(DEBUG "Framebuffer Size: ", (int) init->size);
    putint_phexl(DEBUG "Framebuffer Pitch: ", (int) init->pitch);
}

void framebuffer_clear(fb_pixel_t spec) {
    fb_mbox_init_t *init = ((fb_mbox_init_t*) pi_fb_mbox_ptr);

    for (uint32_t x = 0; x < init->w; x++) {
        for (uint32_t y = 0; y < init->h; y++) {
            uint32_t addr = fb_get_pixel_addr(x, y);
            uint8_t *pix = (uint8_t*) addr;

            *(pix + 0) = spec.r;
            *(pix + 1) = spec.g;
            *(pix + 2) = spec.b;
        }
    }
}
