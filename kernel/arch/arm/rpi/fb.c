#include <liblox/common.h>

#include "fb.h"
#include "mailbox.h"

typedef struct fb_mbox_init_t {
    uint32_t w;
    uint32_t h;
    uint32_t vw;
    uint32_t vh;
    uint32_t pitch;
    uint32_t depth;
    uint32_t x;
    uint32_t y;
    uint32_t pointer;
    uint32_t size;
} packed fb_mbox_init_t;

void framebuffer_init(uint32_t w, uint32_t h) {
    fb_mbox_init_t fb_init = {
        w,
        h,
        w,
        h,
        0,
        32,
        0,
        0,
        0,
        0
    };

    bcm_mailbox_write(1, (uint32_t) &fb_init);
}
