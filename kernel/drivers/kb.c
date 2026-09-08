#include "kb.h"
#include "../stdlib/stdio.h"

keymap_t def_layout = {
    "US-English",
    { 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
      'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
      'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ' },
    { 0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
      'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
      'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ' }
};

static int caps_on  = 0;
static int ctrl     = 0;
static int shift    = 0;
static int extended = 0;

static volatile char     kbuf[KB_BUF_SIZE];
static volatile uint32_t kbuf_head = 0;
static volatile uint32_t kbuf_tail = 0;

void kb_enqueue(char c) {
    if ((kbuf_head - kbuf_tail) >= KB_BUF_SIZE) return;
    kbuf[kbuf_head & (KB_BUF_SIZE - 1)] = c;
    kbuf_head++;
}

int kb_haschar(void) {
    return kbuf_head != kbuf_tail;
}

char kb_getchar(void) {
    while (!kb_haschar())
        asm volatile("hlt");
    char c = kbuf[kbuf_tail & (KB_BUF_SIZE - 1)];
    kbuf_tail++;
    return c;
}

void kb_handler(system_state *sys) {
    (void)sys;
    unsigned char scancode = inb(KB_DATA);

    if (scancode == 0xE0) { extended = 1; return; }

    if (extended) {
        extended = 0;
        switch (scancode) {
            case UP_ARROW:   get_prev_cmd(); break;
            case DOWN_ARROW: get_next_cmd(); break;
            default: break;
        }
        return;
    }

    if (scancode == L_SHIFT || scancode == R_SHIFT) {
        shift = 1;
    } else if (scancode == CAPS_LOCK) {
        caps_on = !caps_on;
    } else if ((scancode & RELEASED) && (scancode == REL_SHIFT_L || scancode == REL_SHIFT_R)) {
        shift = 0;
    } else if (scancode == L_CTRL) {
        ctrl = 1;
    } else if ((scancode & RELEASED) && scancode == REL_CTRL_L) {
        ctrl = 0;
    } else if (!(scancode & RELEASED) && scancode < 128) {
        if (ctrl) {
            switch (def_layout.lower[scancode]) {
                case 'l': shell_clear(); break;
                default: break;
            }
        } else {
            char c = (shift ^ caps_on)
                ? (char)def_layout.upper[scancode]
                : (char)def_layout.lower[scancode];
            if (c) kb_enqueue(c);
        }
    }
}

void init_kb(void) {
    caps_on = 0;
    shift   = 0;
    ctrl    = 0;
    irq_assign_handler(1, kb_handler);
}