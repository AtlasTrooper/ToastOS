#pragma once
#include <stdint.h>

typedef struct rtc_time_t {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint32_t year;
} rtc_time_t;

void rtc_read(rtc_time_t *time);
uint64_t rtc_to_epoch(const rtc_time_t *time);
void init_rtc(void);
uint64_t get_epoch_time(void);