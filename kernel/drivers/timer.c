#include "timer.h"
#include "../io.h"
#include "../serial.h"
#include "../idt/idt.h"
#include "../shell/console.h"

#define PIT_BASE_FREQ     1193180
#define PIT_CHANNEL0_PORT 0x40
#define PIT_COMMAND_PORT  0x43

static volatile uint64_t tick_count    = 0;
static volatile uint32_t channel_0_hz  = 100;

uint64_t get_tick_count(void) {
    return tick_count;
}

uint64_t get_uptime_ms(void) {
    return (tick_count * 1000) / channel_0_hz;
}

uint64_t get_uptime_seconds(void) {
    return tick_count / channel_0_hz;
}

void timer_handler(system_state *sys) {
    (void)sys;
    tick_count++;
    console_tick();
}

void timer_wait_t(uint32_t ticks) {
    uint64_t target = tick_count + (uint64_t)ticks; 
    while (tick_count < target) {
        asm volatile("hlt");
    }
}

void timer_config(uint32_t hz) {
    if (hz == 0) return;
    
    channel_0_hz = hz;
    uint32_t divisor = PIT_BASE_FREQ / hz; 
    
    outb(PIT_COMMAND_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);
    outb(PIT_CHANNEL0_PORT, (divisor >> 8) & 0xFF);
}

void init_timer(void) {
    irq_assign_handler(0, timer_handler);
    timer_config(channel_0_hz);
}