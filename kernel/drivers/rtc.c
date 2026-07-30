#include "rtc.h"
#include "timer.h"
#include "../io.h"

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

static uint64_t boot_epoch_seconds = 0;

static int get_update_in_progress_flag(void) {
    outb(CMOS_ADDR, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

static uint8_t get_rtc_register(uint8_t reg) {
    outb(CMOS_ADDR, reg);
    return inb(CMOS_DATA);
}

void rtc_read(rtc_time_t *time) {
    rtc_time_t last;
    uint8_t regB;

    while (get_update_in_progress_flag());
    time->second = get_rtc_register(0x00);
    time->minute = get_rtc_register(0x02);
    time->hour   = get_rtc_register(0x04);
    time->day    = get_rtc_register(0x07);
    time->month  = get_rtc_register(0x08);
    time->year   = get_rtc_register(0x09);

    //double validation method
    do {
        last = *time;
        while (get_update_in_progress_flag());
        time->second = get_rtc_register(0x00);
        time->minute = get_rtc_register(0x02);
        time->hour   = get_rtc_register(0x04);
        time->day    = get_rtc_register(0x07);
        time->month  = get_rtc_register(0x08);
        time->year   = get_rtc_register(0x09);
    } while (last.second != time->second || last.minute != time->minute ||
             last.hour   != time->hour   || last.day    != time->day    ||
             last.month  != time->month  || last.year   != time->year);

    regB = get_rtc_register(0x0B);

    // Bcd to bin
    if (!(regB & 0x04)) {
        time->second = (time->second & 0x0F) + ((time->second / 16) * 10);
        time->minute = (time->minute & 0x0F) + ((time->minute / 16) * 10);
        time->hour   = ((time->hour & 0x0F) + (((time->hour & 0x70) / 16) * 10)) | (time->hour & 0x80);
        time->day    = (time->day & 0x0F) + ((time->day / 16) * 10);
        time->month  = (time->month & 0x0F) + ((time->month / 16) * 10);
        time->year   = (time->year & 0x0F) + ((time->year / 16) * 10);
    }

    //12 to 24 convertion
    if (!(regB & 0x02) && (time->hour & 0x80)) {
        time->hour = ((time->hour & 0x7F) + 12) % 24;
    }

    time->year += 2000;
}

uint64_t rtc_to_epoch(const rtc_time_t *t) {
    uint32_t y = t->year;
    uint32_t m = t->month;
    uint32_t d = t->day;

    if (m <= 2) {
        y -= 1;
        m += 12;
    }

    uint64_t days = (365 * y) + (y / 4) - (y / 100) + (y / 400) + (306 * (m + 1) / 10) + d - 719591;
    return (days * 86400) + (t->hour * 3600) + (t->minute * 60) + t->second;
}

void init_rtc(void) {
    rtc_time_t now;
    rtc_read(&now);
    boot_epoch_seconds = rtc_to_epoch(&now);
}

uint64_t get_epoch_time(void) {
    return boot_epoch_seconds + get_uptime_seconds();
}