#pragma once
#include <stdint.h>
#include <stddef.h>
#define PSF2_MAGIC 0x864ab572

typedef struct {
    uint32_t magic;         /* magic bytes to identify PSF */
    uint32_t version;       /* zero */
    uint32_t headersize;    /* offset of bitmaps in file, 32 */
    uint32_t flags;         /* 0 if there's no unicode table */
    uint32_t numglyph;      /* number of glyphs */
    uint32_t bytesperglyph; /* size of each glyph */
    uint32_t height;        /* height in pixels */
    uint32_t width;         /* width in pixels */
} PSF2_Header;

/*
will implement later when 64 bit migration
is complete and we have dynamic memory allocation back.
*/
void psf_init();
uint32_t font_height(void);
uint32_t font_width(void);
void font_draw_char(char c, uint32_t px, uint32_t py, uint32_t fg, uint32_t bg);
