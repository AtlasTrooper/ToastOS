#include "font.h"
#include "framebuffer.h"

extern char _binary_kernel_fonts_font_psf_start[];
extern char _binary_kernel_fonts_font_psf_end[];

static PSF2_Header* hdr = (PSF2_Header *)_binary_kernel_fonts_font_psf_start;

uint32_t font_width(void) {
    return hdr->width;
}
 
uint32_t font_height(void) {
    return hdr->height;
}

void font_draw_char(char c, uint32_t px, uint32_t py, uint32_t fg, uint32_t bg) { 
    if (hdr->magic != PSF2_MAGIC) return;
 
    uint32_t glyph_index = (uint32_t)(unsigned char)c;
    if (glyph_index >= hdr->numglyph) glyph_index = 0;
 
    uint8_t *glyph = (uint8_t *)&_binary_kernel_fonts_font_psf_start
                     + hdr->headersize
                     + glyph_index * hdr->bytesperglyph;
 
    uint32_t bytes_per_line = (hdr->width + 7) / 8;
 
    for (uint32_t row = 0; row < hdr->height; row++) {
        for (uint32_t col = 0; col < hdr->width; col++) {
            
            uint8_t byte = glyph[row * bytes_per_line + col / 8];
            uint8_t bit  = byte & (0x80 >> (col % 8));
            fb_put_pixel(px + col, py + row, bit ? fg : bg);
        }
    }
}
 

