#pragma once
#include <stdint.h>
#include "../idt/idt.h"
#include "../multitasking/thread.h"

void timer_handler(system_state *sys);
void timer_wait_t(uint32_t ticks);
void init_timer(void);
void timer_config(uint32_t hz);

uint64_t get_tick_count(void);
uint64_t get_uptime_ns(void);
uint64_t get_uptime_ms(void);
uint64_t get_uptime_seconds(void); 