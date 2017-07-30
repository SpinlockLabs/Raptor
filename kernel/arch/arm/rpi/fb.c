#include <liblox/common.h>
#include <liblox/io.h>
#include <liblox/hex.h>

#include "fb.h"
#include "mailbox.h"
#include "mmio.h"
#include "board.h"

static uint8_t* pi_framebuffer;
static uint32_t fb_w = 640;
static uint32_t fb_h = 480;
static uint32_t fb_pitch = 3;

static uint32_t fb_get_pixel_addr(uint32_t x, uint32_t y) {
    uint32_t offset = ((y * fb_pitch) + (x * 3));
    return (uint32_t) ((uint32_t) pi_framebuffer + offset);
}

void framebuffer_init(uint32_t w, uint32_t h) {
    fb_w = w;
    fb_h = h;

    mmio_write(PI_FIRMWARE_FRAMEBUFFER + 0x00, w);
    mmio_write(PI_FIRMWARE_FRAMEBUFFER + 0x04, h);
    mmio_write(PI_FIRMWARE_FRAMEBUFFER + 0x08, w);
    mmio_write(PI_FIRMWARE_FRAMEBUFFER + 0x0C, h);
    mmio_write(PI_FIRMWARE_FRAMEBUFFER + 0x10, 0); // Pitch
    mmio_write(PI_FIRMWARE_FRAMEBUFFER + 0x14, 32); // Depth
    mmio_write(PI_FIRMWARE_FRAMEBUFFER + 0x18, 0); // X
    mmio_write(PI_FIRMWARE_FRAMEBUFFER + 0x1C, 0); // Y
    mmio_write(PI_FIRMWARE_FRAMEBUFFER + 0x20, 0); // Buffer
    mmio_write(PI_FIRMWARE_FRAMEBUFFER + 0x24, 0); // Size

    bcm_mailbox_write(1, PI_FIRMWARE_FRAMEBUFFER);
    uint32_t result = bcm_mailbox_read(1);
    printf(DEBUG "Framebuffer Result: 0x%x\n", result);

    for (uint i = 0; i < 10; i++) {
        uint32_t addr = (PI_FIRMWARE_FRAMEBUFFER) + (i * 4);
        uint32_t value = mmio_read(addr);
        printf(DEBUG "Framebuffer Info: 0x%x = 0x%x (%d)\n", addr, value, value);
    }

    pi_framebuffer = (uint8_t*) mmio_read(PI_FIRMWARE_FRAMEBUFFER + 0x20);

    framebuffer_clear(255, 0, 0);

    uint32_t size = mmio_read(PI_FIRMWARE_FRAMEBUFFER + 0x24);
    uint32_t pitch = mmio_read(PI_FIRMWARE_FRAMEBUFFER + 0x10);

    printf(DEBUG "Framebuffer Location: 0x%x\n", pi_framebuffer);
    printf(DEBUG "Framebuffer Size: 0x%x\n", size);
    printf(DEBUG "Framebuffer Pitch: 0x%x\n", pitch);
}

void framebuffer_clear(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t w = fb_w;
    uint32_t h = fb_h;

    for (uint32_t x = 0; x < w; x++) {
        for (uint32_t y = 0; y < h; y++) {
            uint32_t addr = fb_get_pixel_addr(x, y);

            mmio_write(addr + 0, r);
            mmio_write(addr + 1, g);
            mmio_write(addr + 3, b);
        }
    }
}
