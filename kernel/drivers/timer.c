#include "timer.h"
#include "../io.h"
#include "../serial.h"
#include "../idt/idt.h"

#define INIT_FREQ 1193180

static volatile int tick_count    = 0;
static volatile int channel_0_hz  = 100;

int get_tick_count(void) {
    return tick_count;
}

void timer_handler(system_state *sys) {
    (void)sys;
    tick_count++;
}

void timer_wait_t(int ticks) {
    int future = tick_count + ticks;
    while (tick_count < future)
        asm volatile("hlt");
}

void timer_config(int hz) {
    int divisor = INIT_FREQ / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

void init_timer(void) {
    irq_assign_handler(0, timer_handler);
    timer_config(channel_0_hz);
}

void speaker_config(uint32_t freq) {
    uint32_t div = INIT_FREQ / freq;
    outb(0x43, 0xb6);
    outb(0x42, div & 0xFF);
    outb(0x42, div >> 8);
}

void enable_speaker(void) {
    uint8_t tmp = inb(0x61);
    if (tmp != (tmp | 3))
        outb(0x61, tmp | 3);
}

void disable_speaker(void) {
    uint8_t off = inb(0x61) & 0xFC;
    outb(0x61, off);
}

void beep(void) {
    enable_speaker();
    timer_wait_t(100);
    disable_speaker();
}

void play_note(uint32_t freq, uint32_t dur) {
    int gap = 10;
    if (freq == 0) {
        timer_wait_t(dur);
    } else {
        speaker_config(freq);
        enable_speaker();
        timer_wait_t(dur - gap);
        disable_speaker();
        timer_wait_t(gap);
    }
}

void play_sandstorm(void) {
    timer_config(1000);
    int note_len = 90;
    int i;
    for (i = 0; i < 5; i++) play_note(NOTE_B4, note_len);
    timer_wait_t(note_len);
    for (i = 0; i < 7; i++) play_note(NOTE_B4, note_len);
    timer_wait_t(note_len);
    play_note(NOTE_D5, note_len);
    for (i = 0; i < 7; i++) play_note(NOTE_B4, note_len);
    for (i = 0; i < 7; i++) play_note(NOTE_E5, note_len);
    play_note(NOTE_A4, note_len);
    for (i = 0; i < 7; i++) play_note(NOTE_B4, note_len);
    timer_config(100);
}