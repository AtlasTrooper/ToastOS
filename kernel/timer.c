#include "timer.h"
#include "vga.h"
#include "serial.h"
#include "io.h"
#define INIT_FREQ 1193180
int tick_count = 0;
int channel_0_hz = 100;

void timer_handler(system_state* sys){    
    tick_count ++;
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


void play_note(uint32_t freq, uint32_t dur) {
    int gap = 10;        // 10ms gap for crisp staccato
    if (freq == 0) {
        timer_wait_t(dur);
    } else {
        speaker_config(freq);
        enable_speaker();
        timer_wait_t(dur - gap); // Play for 80ms
        disable_speaker();
        timer_wait_t(gap);       // Silent for 10ms
    }
}

void play_sandstorm() {
    timer_config(1000); // accelerate timer for 1ms intervals

    int note_len = 90;
    int i;

    for(i = 0; i < 5; i++) play_note(NOTE_B4, note_len);
    timer_wait_t(note_len); 
    
    for(i = 0; i < 7; i++) play_note(NOTE_B4, note_len);
    timer_wait_t(note_len);

    play_note(NOTE_D5, note_len);
    for(i = 0; i < 7; i++) play_note(NOTE_B4, note_len);
    
    for(i = 0; i < 7; i++) play_note(NOTE_E5, note_len);
    
    play_note(NOTE_A4, note_len);
    for(i = 0; i < 7; i++) play_note(NOTE_B4, note_len);

    timer_config(100); //return timer to standard speeds
}

