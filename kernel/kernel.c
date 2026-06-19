#include <stdint.h>
#include <stddef.h>
#include "limine.h"

// Limine requests — compiler must not optimise these away
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

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    // Draw a white diagonal line — proof of life
    for (size_t i = 0; i < 100; i++) {
        uint32_t *fb_ptr = fb->address;
        fb_ptr[i * (fb->pitch / 4) + i] = 0xffffff;
    }

    hcf();
}