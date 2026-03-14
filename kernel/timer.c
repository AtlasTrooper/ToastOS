#include "timer.h"
#include "vga.h"
#include "io.h"

int tick_count = 0;
int channel_0_hz = 100;

void timer_handler(system_state* sys){
    
    tick_count ++;
    // if(tick_count %channel_0_hz ==0){
    //     putstr("One mississippi\n");
    // } 
   
}
void timer_wait_t(unsigned int ticks){
    unsigned long future_ticks;
    future_ticks = tick_count + ticks;
    while(tick_count < future_ticks);
}
void timer_wait_s(unsigned int seconds){
    int count = seconds;
    while(count !=0){
        if(tick_count %18==0){
            count -=1;
        }
    }
}

void timer_config(int hz){
    int divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);   
    outb(0x40, divisor >> 8);  
}

void init_timer(){
    irq_assign_handler(0, timer_handler);
    timer_config(channel_0_hz);
}
