#include "interrupts/idt.h"
void timer_handler(system_state *sys);
void timer_wait_t(unsigned int ticks);
void timer_wait_s(unsigned int seconds);
void init_timer();
void timer_config(int hz);