#include "timer.h"
#include "vga.h"
#include "serial.h"
#include "io.h"
#define INIT_FREQ 1193180
int tick_count = 0;
int channel_0_hz = 100;

void timer_handler(system_state* sys){
    
    tick_count ++;
    // if(tick_count %channel_0_hz ==0){
    //     putstr("One mississippi\n");
    // } 
   
}
void timer_wait_t(int ticks){
    unsigned long future_ticks;
    future_ticks = tick_count + ticks;
    while(tick_count < future_ticks){
        asm volatile("hlt");
    }
}

void timer_config(int hz){
    int divisor = INIT_FREQ / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);   
    outb(0x40, divisor >> 8);  
}

void init_timer(){
    irq_assign_handler(0, timer_handler);
    timer_config(channel_0_hz);
}

void speaker_config(uint32_t freq){
    uint32_t div;
    div = INIT_FREQ / freq;

    outb(0x43, 0xb6);
    outb(0x42, div & 0xFF);
    outb(0x42, div >> 8);
}

void enable_speaker(){
    uint8_t tmp;
    tmp = inb(0x61); //checks if we are in "in" or "out"
    if(tmp != (tmp | 3)){
        outb(0x61, tmp | 3);
    }
}

void beep(){
    //debug_print("beepin\n");
    enable_speaker();
    timer_wait_t(100);
    disable_speaker();
    //debug_print("disabled\n");

}

void disable_speaker(){
    uint8_t off = inb(0x61)&0xFC;
    outb(0x61, off);
}

void play_note(uint32_t freq, uint32_t ticks) {
    if (freq == 0) {
        disable_speaker();
        timer_wait_t(ticks);
    } else {
        speaker_config(freq);
        enable_speaker();
        
        // Use 80% of the time for the sound, 20% for the silence gap
        // This ensures the notes don't "blur" together as you slow down
        uint32_t sound_duration = (ticks * 8) / 10;
        uint32_t gap_duration = ticks - sound_duration;

        timer_wait_t(sound_duration);
        disable_speaker();
        timer_wait_t(gap_duration);
    }
}

void play_sandstorm() {
    // Increase 'tempo_ticks' to slow it down.
    // 9 = original speed (~167 BPM)
    // 12 = moderately slow
    // 15 = very chilled out
    int tempo_ticks = 13; 
    int i;

    // Part 1: The Build
    for(i = 0; i < 5; i++) play_note(NOTE_B4, tempo_ticks);
    timer_wait_t(tempo_ticks); // Half-note rest
    
    for(i = 0; i < 7; i++) play_note(NOTE_B4, tempo_ticks);
    timer_wait_t(tempo_ticks);

    // Part 2: The Pitch Shifts
    play_note(NOTE_D5, tempo_ticks);
    for(i = 0; i < 7; i++) play_note(NOTE_B4, tempo_ticks);
    
    play_note(NOTE_E5, tempo_ticks);
    for(i = 0; i < 6; i++) play_note(NOTE_E5, tempo_ticks);
    
    play_note(NOTE_A4, tempo_ticks);
    for(i = 0; i < 7; i++) play_note(NOTE_B4, tempo_ticks);
}

