#include "console.h"
#include "../framebuffer.h"
#include "../font.h"

static int ansi_state = 0;
static int ansi_param = 0;

#define CURSOR_BLINK_TICKS 50
static volatile int cursor_visible = 0;
static volatile int cursor_blink_counter = 0;

static uint32_t con_fg = CON_WHITE;
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
    if (ansi_state > 0) {
        if (ansi_state == 1 && c == '[') {
            ansi_state = 2; // Entered '[', start reading numbers
            ansi_param = 0;
            return;
        } else if (ansi_state == 2 && c >= '0' && c <= '9') {
            ansi_param = ansi_param * 10 + (c - '0'); // Accumulate digit
            return;
        } else if (ansi_state == 2 && c == 'm') {
            // Sequence ended, apply the color based on ansi_param
            uint32_t ansi_colors[] = {
                CON_BLACK, CON_RED, CON_GREEN, CON_YELLOW, 
                CON_BLUE, CON_MAGENTA, CON_CYAN, CON_WHITE
            };

            if (ansi_param == 0) {
                // Reset code (\033[0m)
                con_fg = CON_WHITE;
                con_bg = CON_BLACK;
            } else if (ansi_param >= 30 && ansi_param <= 37) {
                // FG color codes (30-37)
                con_fg = ansi_colors[ansi_param - 30];
            } else if (ansi_param >= 40 && ansi_param <= 47) {
                // BG color codes (40-47)
                con_bg = ansi_colors[ansi_param - 40];
            }
            
            ansi_state = 0;
            return;
        } else {
            ansi_state = 0;
            return;
        }
    }

    if (c == '\033') {
        ansi_state = 1;
        return;
    }

    switch (c) {
        case '\n':
            newLine();
            break;

        case '\r':
            cursor_col = 0;
            break;

        case '\b':
            if (cursor_visible) draw_char_at(' ', cursor_col, cursor_row);
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

void console_tick() {
    cursor_blink_counter ++;
    if (cursor_blink_counter < CURSOR_BLINK_TICKS) {
        return;
    }

    cursor_blink_counter = 0;

    uint32_t px = cursor_col * font_width();
    uint32_t py = cursor_row * font_height();
    uint32_t w  = font_width();
    uint32_t h  = font_height();

    if(cursor_visible) {
        fb_draw_rect(px, py, w, h, con_bg);
        cursor_visible = 0;
    } else {
        draw_char_at('_', cursor_col, cursor_row);
        cursor_visible = 1;
    }
}