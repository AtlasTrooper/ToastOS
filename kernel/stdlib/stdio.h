#pragma once
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

void putchar(char c);
void putstr(const char *s);
void printf(const char *fmt, ...);
void vprintf(const char *fmt, va_list args);

/*
TODO: Dlmalloc requires some additions to be made to stdio:
1. errno
2. fprintf
3. ENOMEM
4. stderr

*/