#include <stdint.h>
#include <stddef.h>
#include "limine.h"

// Limine requests — compiler must not optimise these away
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// GCC may generate calls to these even in freestanding mode — must exist
void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d = dest; const uint8_t *s = src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dest;
}
void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;
    for (size_t i = 0; i < n; i++) p[i] = (uint8_t)c;
    return s;
}
void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *d = dest; const uint8_t *s = src;
    if (s > d)      for (size_t i = 0; i < n; i++) d[i] = s[i];
    else if (s < d) for (size_t i = n; i > 0; i--) d[i-1] = s[i-1];
    return dest;
}
int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *a = s1, *b = s2;
    for (size_t i = 0; i < n; i++)
        if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
    return 0;
}

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