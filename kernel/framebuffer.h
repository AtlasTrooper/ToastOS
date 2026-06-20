#pragma once
#include <stdint.h>
#include <stddef.h>
#include "limine.h"
#define COLOR_BLACK   0x00000000
#define COLOR_WHITE   0x00FFFFFF
#define COLOR_RED     0x00FF0000
#define COLOR_GREEN   0x0000FF00
#define COLOR_BLUE    0x000000FF
#define COLOR_CYAN    0x0000FFFF
#define COLOR_MAGENTA 0x00FF00FF
#define COLOR_YELLOW  0x00FFFF00
#define COLOR_GREY    0x00808080

typedef struct {
    uint32_t *addr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
} Framebuffer;

void fb_init(struct limine_framebuffer *limine_fb); 
void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void fb_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void fb_clear(uint32_t color);

const Framebuffer *fb_get(void);
