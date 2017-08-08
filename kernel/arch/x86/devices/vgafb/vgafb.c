#include "vgafb.h"

#include <kernel/arch/x86/io.h>
#include <kernel/arch/x86/devices/pci/pci.h>
#include <kernel/arch/x86/paging.h>
#include <kernel/graphics/fb.h>
#include <kernel/cmdline.h>
#include <liblox/string.h>
#include <liblox/memory.h>

typedef struct vgafb vgafb_t;

typedef void (*vgafb_set_resolution_t)(vgafb_t*);

struct vgafb {
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t depth;
    uint8_t* memory;
    vgafb_set_resolution_t set_resolution;
};

static void bochs_scan_pci(uint32_t device, uint16_t v, uint16_t d, void* extra) {
    if ((v == 0x1234 && d == 0x1111) ||
        (v == 0x80EE && d == 0xBEEF)) {
        uintptr_t t = pci_read_field(device, PCI_BAR0, 4);
        if (t > 0) {
            *((uint8_t**) extra) = (uint8_t *)(t & 0xFFFFFFF0);
        }
    }
}

static void vgafb_init(vgafb_t* vga) {
    framebuffer_t* fb = framebuffer_create("vgafb");
    fb->width = vga->width;
    fb->height = vga->height;
    fb->buffer = vga->memory;
    fb->pitch = vga->stride;
    fb->format = PIXEL_FMT_RGBA32;
    framebuffer_register(device_root(), fb);
}

#define PREFERRED_VY 4096
#define PREFERRED_B 32

static void res_change_bochs(
    vgafb_t* vga,
    uint32_t x,
    uint32_t y
) {
    outs(0x1CE, 0x04);
    outs(0x1CF, 0x00);
    /* Uh oh, here we go. */
    outs(0x1CE, 0x01);
    outs(0x1CF, (uint16_t) x);
    /* Set Y resolution to 768 */
    outs(0x1CE, 0x02);
    outs(0x1CF, (uint16_t) y);
    /* Set bpp to 32 */
    outs(0x1CE, 0x03);
    outs(0x1CF, PREFERRED_B);
    /* Set Virtual Height to stuff */
    outs(0x1CE, 0x07);
    outs(0x1CF, PREFERRED_VY);
    /* Turn it back on */
    outs(0x1CE, 0x04);
    outs(0x1CF, 0x41);

    /* Read X to see if it's something else */
    outs(0x1CE, 0x01);
    uint16_t new_x = ins(0x1CF);
    if (x != new_x) {
        x = new_x;
    }

    vga->width = x;
    vga->stride = (uint32_t) (x * 4);
    vga->height = y;
}

static void graphics_install_bochs(
    vgafb_t* vga,
    uint32_t width,
    uint32_t height) {
    uint32_t vid_memsize;
    outs(0x1CE, 0x00);

    uint16_t i = ins(0x1CF);
    if (i < 0xB0C0 || i > 0xB0C6) {
        printf(WARN "[VGA FB] Bochs graphics failed to initialize. Was it already initialized?\n");
        return;
    }

    outs(0x1CF, 0xB0C4);
    i = ins(0x1CF);
    res_change_bochs(vga, width, height);
    width = vga->width;

    pci_scan(bochs_scan_pci, -1, &vga->memory);
    if (vga->memory != NULL) {
        uintptr_t fb_offset = (uintptr_t) vga->memory;
        for (uintptr_t n = fb_offset; n <= fb_offset + 0xFF0000; n += 0x1000) {
            paging_map_dma(n, n);
        }

        goto mem_found;
    } else {
        uint32_t* text_vid_mem = (uint32_t*) 0xA0000;
        text_vid_mem[0] = 0xA5ADFACE;

        for (uintptr_t fb_offset = 0xE0000000; fb_offset < 0xFF000000; fb_offset += 0x01000000) {
            /* Enable the higher memory */
            for (uintptr_t x = fb_offset; x <= fb_offset + 0xFF0000; x += 0x1000) {
                paging_map_dma(x, x);
            }

            /* Go find it */
            for (uintptr_t x = fb_offset; x < fb_offset + 0xFF0000; x += 0x1000) {
                if (((uintptr_t*) x)[0] == 0xA5ADFACE) {
                    vga->memory = (uint8_t*) x;
                    goto mem_found;
                }
            }
        }
    }

mem_found:
    outs(0x1CE, 0x0a);
    i = ins(0x1CF);

    if (i > 1) {
        vid_memsize = (uint32_t) i * 64 * 1024;
    } else {
        vid_memsize = inl(0x1CF);
    }

    printf(DEBUG "[VGA FB] Memory is of size %d\n", vid_memsize);

    for (uintptr_t a = (uintptr_t) vga->memory; a <= (uintptr_t) vga->memory + vid_memsize; a += 0x1000) {
        paging_map_dma(a, a);
    }

    vga->width = width;
    vga->height = height;
    vga->depth = PREFERRED_B;
    vga->stride = vga->width * 4;

    vgafb_init(vga);
}

static void graphics_install_preset(
    vgafb_t* vga,
    uint32_t width,
    uint32_t height
) {
    uint16_t b = 32;

    /* Hack to find video memory. */
    uint32_t* herp = (uint32_t*) 0xA0000;
    herp[0] = 0xA5ADFACE;
    herp[1] = 0xFAF42943;

    if (vga->memory != NULL) {
        uintptr_t fb_offset = (uintptr_t) vga->memory;
        for (uintptr_t n = fb_offset; n <= fb_offset + 0xFF0000; n += 0x1000) {
            paging_map_dma(n, n);
        }

        if (((uintptr_t*) vga->memory)[0] == 0xA5ADFACE &&
            ((uintptr_t*) vga->memory)[1] == 0xFAF42943) {
            goto mem_found;
        }
    }

    for (uint i = 2; i < 1000; i += 2) {
        herp[i] = 0xFF00FF00;
        herp[i + 1] = 0x00FF00FF;
    }

    for (uintptr_t fb_offset = 0xE0000000; fb_offset < 0xFF000000; fb_offset += 0x01000000) {
        for (uintptr_t i = fb_offset; i <= fb_offset + 0xFF0000; i += 0x1000) {
            paging_map_dma(i, i);
        }

        for (uintptr_t x = fb_offset; x < fb_offset + 0xFF0000; x += 0x1000) {
            if (((uintptr_t*) x)[0] == 0xA5ADFACE && ((uintptr_t*) x)[1] == 0xFAF42943) {
                vga->memory = (uint8_t*) x;
                goto mem_found;
            }
        }
    }

    printf(WARN "[VGA FB] Failed to find video memory. This could end badly.\n");
mem_found:
    vga->width = width;
    vga->height = height;
    vga->depth = b;
    vga->stride = width * 4;
    vgafb_init(vga);
}

void vgafb_setup(void) {
    char* vid = cmdline_read("vgafb");
    if (!vid) {
        return;
    }

    printf(INFO "[VGA FB] Initializing video with '%s'\n", vid);

    vgafb_t* vga = zalloc(sizeof(vgafb_t));
    if (strcmp(vid, "bochs") == 0) {
        graphics_install_bochs(vga, 1024, 768);
    } else if (strcmp(vid, "preset") == 0) {
        graphics_install_preset(vga, 640, 480);
    } else {
        printf(WARN "[VGA FB] Unknown video backend '%s'\n", vid);
        free(vga);
    }
}
