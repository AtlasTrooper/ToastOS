#include "framebuffer.h"

static Framebuffer fb;

void fb_init(struct limine_framebuffer *limine_fb) {
    fb.addr = (uint32_t*)limine_fb->address;
    fb.width = (uint32_t)limine_fb->width;
    fb.height = (uint32_t)limine_fb->height;
    fb.pitch = (uint32_t)limine_fb->pitch;
    fb.bpp = limine_fb->bpp;
}

void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= fb.width || y >= fb.height) return;
    uint32_t *row = (uint32_t *)((uint8_t *)fb.addr + y * fb.pitch);
    row[x] = color;
}

void fb_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t colour) {
    for (uint32_t row = y; row < y + h && row < fb.height; row++) {
        for (uint32_t col = x; col < x + w && col < fb.width; col++) {
            uint32_t *line = (uint32_t *)((uint8_t *)fb.addr + row * fb.pitch);
            line[col] = colour;
        }
    }
}

void fb_clear(uint32_t color) {
    for (uint32_t y = 0; y < fb.height; y++) {
        uint32_t *row = (uint32_t *)((uint8_t *)fb.addr + y * fb.pitch);
        for (uint32_t x = 0; x < fb.width; x++) {
            row[x] = color;
        }
    }
}

const Framebuffer* fb_get(void) {
    return &fb;
}
