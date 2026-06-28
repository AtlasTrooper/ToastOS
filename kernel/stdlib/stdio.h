#pragma once
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

void putchar(char c);
void putstr(const char *s);
void printf(const char *fmt, ...);
void vprintf(const char *fmt, va_list args);