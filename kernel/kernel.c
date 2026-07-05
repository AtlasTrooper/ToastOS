#include <stdint.h>
#include <stddef.h>
#include "mmu/memory.h"
#include "limine.h"
#include "framebuffer.h"
#include "font.h"
#include "shell/console.h"
#include "stdlib/stdio.h"
#include "stdlib/string.h"
#include "io.h"
#include "serial.h"
#include "gdt/gdt.h"
#include "idt/idt.h"
#include "drivers/timer.h"
#include "drivers/kb.h"
#include "shell/tsh.h"
#include "mmu/pmm.h"
#include "mmu/vmm.h"
#include "mmu/heap.h"
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

    debug_print("\n[Hello from the 64 bit serial port!]\n");

    initGDT();
    initIDT();
    init_timer();
    kb_init();
    init_pmm();
    init_vmm();
    init_kheap();
    init_shell();

}