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

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    fb_init(limine_framebuffer);
    fb_clear(COLOR_BLACK);

    hcf();
}