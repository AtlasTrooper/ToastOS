#include "tsh.h"
#include "../framebuffer.h"
#include "../font.h"

static uint32_t con_fg = CON_LIGHT_GREY;
static uint32_t con_bg = CON_BLACK;
static uint32_t cursor_col = 0;
static uint32_t cursor_row = 0;
static uint32_t con_cols = 0;
static uint32_t con_rows = 0;

void console_init(void) {
    const Framebuffer *fb = fb_get();
    con_cols = fb->width  / font_width();
    con_rows = fb->height / font_height();
    cursor_col = 0;
    cursor_row = 0;
    terminal_clear();
}

void console_set_color(uint32_t fg, uint32_t bg) {
    con_fg = fg;
    con_bg = bg;
}

static void draw_char_at(char c, uint32_t col, uint32_t row) {
    font_draw_char(c,
                   col * font_width(),
                   row * font_height(),
                   con_fg, con_bg);
}

static void clear_row(uint32_t row) {
    for (uint32_t col = 0; col < con_cols; col++) {
        draw_char_at(' ', col, row);
    }
}

void terminal_scroll(void) {
    const Framebuffer *fb = fb_get();
    uint32_t glyph_h = font_height();
    uint32_t copy_rows = fb->height - glyph_h;

    uint8_t *base = (uint8_t *)fb->addr;
    for (uint32_t y = 0; y < copy_rows; y++) {
        uint32_t *dst = (uint32_t *)(base + y             * fb->pitch);
        uint32_t *src = (uint32_t *)(base + (y + glyph_h) * fb->pitch);
        for (uint32_t x = 0; x < fb->width; x++) {
            dst[x] = src[x];
        }
    }

    cursor_row = con_rows - 1;
    cursor_col = 0;
    clear_row(cursor_row);
}

void newLine(void) {
    cursor_col = 0;
    if (cursor_row < con_rows - 1) {
        cursor_row++;
    } else {
        terminal_scroll();
    }
}

void clearLine(size_t row) {
    if (row >= con_rows) return;
    clear_row((uint32_t)row);
    if ((uint32_t)row == cursor_row) cursor_col = 0;
}

void clearCurrentLine(void) {
    clearLine(cursor_row);
}

void terminal_clear(void) {
    fb_clear(con_bg);
    cursor_col = 0;
    cursor_row = 0;
}

void putchar(char c) {
    switch (c) {
        case '\n':
            newLine();
            break;

        case '\r':
            cursor_col = 0;
            break;

        case '\b':
            if (cursor_col == 0 && cursor_row != 0) {
                cursor_row--;
                cursor_col = con_cols;
            }
            if (cursor_col > 0) {
                cursor_col--;
                draw_char_at(' ', cursor_col, cursor_row);
            }
            break;

        case '\t': {
            if (cursor_col == con_cols) newLine();
            uint32_t tab = 4 - (cursor_col % 4);
            while (tab-- && cursor_col < con_cols) {
                draw_char_at(' ', cursor_col++, cursor_row);
            }
            break;
        }

        default:
            if (cursor_col == con_cols) newLine();
            draw_char_at(c, cursor_col++, cursor_row);
            break;
    }
}

void putstr(const char *str) {
    while (*str) putchar(*str++);
}