#pragma once
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

void putchar(char c);
void printf(const char *fmt, ...);
void putstr(const char *str);

#define PRINTF_STATE_START  0
#define PRINTF_STATE_LENGTH 1
#define PRINTF_STATE_SHORT  2
#define PRINTF_STATE_LONG   3
#define PRINTF_STATE_SPEC   4

#define PRINTF_LENGTH_START      0
#define PRINTF_LENGTH_SHORT_SHORT 1
#define PRINTF_LENGTH_SHORT      2
#define PRINTF_LENGTH_LONG       3
#define PRINTF_LENGTH_LONG_LONG  4