#include <stdint.h>
#include <stddef.h>
#include "mmu/memory.h"
#include "limine.h"
#include "framebuffer.h"
#include "font.h"

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static void hcf(void) {
    asm("cli");
    for (;;) asm("hlt");
}

void _start(void) {
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1)
        hcf();

    struct limine_framebuffer *fb =
     framebuffer_request.response->framebuffers[0];

    fb_init(fb);
    fb_clear(COLOR_BLACK);

    const char *msg = "ToastOS x86_64 - Framebuffer OK!";
    uint32_t x = 16;
    for (int i = 0; msg[i] != '\0'; i++) {
        font_draw_char(msg[i], x, 16, COLOR_WHITE, COLOR_BLACK);
        x += font_width();
    }


    hcf();
}