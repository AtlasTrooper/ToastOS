#include <stdint.h>
#include <stddef.h>
#include "mmu/memory.h"
#include "limine.h"
#include "framebuffer.h"
#include "font.h"
#include "shell/tsh.h"
#include "stdlib/stdio.h"

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
    console_init();
    console_set_color(CON_WHITE, CON_BLACK);

    printf("ToastOS x86_64 - framebuffer console OK\n");
    printf("cols: %d  rows: %d\n",
           fb_get()->width  / font_width(),
           fb_get()->height / font_height());
 
    console_set_color(CON_LIGHT_GREEN, CON_BLACK);
    putstr("tsh> ");
    console_set_color(CON_LIGHT_GREY, CON_BLACK);


    hcf();
}