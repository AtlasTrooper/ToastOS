#include "interrupts/idt.h"
#define NOTE_B4  494
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_A4  440
void timer_handler(system_state *sys);

void timer_wait_t(int ticks);
void init_timer();
void timer_config(int hz);

void speaker_config(uint32_t freq);
void enable_speaker();
void beep();
void disable_speaker();

void play_note(uint32_t freq, uint32_t ticks);
void play_sandstorm();