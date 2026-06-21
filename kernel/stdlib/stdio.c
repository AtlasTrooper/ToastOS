#include "stdio.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "../shell/tsh.h"

static const char possibleChars[] = "0123456789abcdef";

static void printf_number(uint64_t number, bool sign, int64_t signed_val, int radix) {
    char buffer[64];
    int pos = 0;
    bool negative = false;

    if (sign && signed_val < 0) {
        negative = true;
        number = (uint64_t)(-signed_val);
    }

    do {
        buffer[pos++] = possibleChars[number % radix];
        number /= radix;
    } while (number > 0);

    if (negative) buffer[pos++] = '-';

    while (--pos >= 0) putchar(buffer[pos]);
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int state  = PRINTF_STATE_START;
    int length = PRINTF_LENGTH_START;
    int radix  = 10;
    bool sign  = false;

    while (*fmt) {
        switch (state) {
        case PRINTF_STATE_START:
            if (*fmt == '%') state = PRINTF_STATE_LENGTH;
            else             putchar(*fmt);
            break;

        case PRINTF_STATE_LENGTH:
            if (*fmt == 'h') {
                length = PRINTF_LENGTH_SHORT;
                state  = PRINTF_STATE_SHORT;
                break;
            } else if (*fmt == 'l') {
                length = PRINTF_LENGTH_LONG;
                state  = PRINTF_STATE_LONG;
                break;
            }
            goto SPEC;

        case PRINTF_STATE_SHORT:
            if (*fmt == 'h') { length = PRINTF_LENGTH_SHORT_SHORT; state = PRINTF_STATE_SPEC; break; }
            goto SPEC;

        case PRINTF_STATE_LONG:
            if (*fmt == 'l') { length = PRINTF_LENGTH_LONG_LONG;   state = PRINTF_STATE_SPEC; break; }
            goto SPEC;

        case PRINTF_STATE_SPEC:
        SPEC:
            switch (*fmt) {
            case 'c':
                putchar((char)va_arg(args, int));
                break;

            case 's':
                putstr(va_arg(args, const char *));
                break;

            case '%':
                putchar('%');
                break;

            case 'd': case 'i':
                sign = true; radix = 10;
                if (length == PRINTF_LENGTH_LONG_LONG) {
                    long long v = va_arg(args, long long);
                    printf_number((uint64_t)v, true, (int64_t)v, radix);
                } else if (length == PRINTF_LENGTH_LONG) {
                    long v = va_arg(args, long);
                    printf_number((uint64_t)v, true, (int64_t)v, radix);
                } else {
                    int v = va_arg(args, int);
                    printf_number((uint64_t)v, true, (int64_t)v, radix);
                }
                break;

            case 'u':
                sign = false; radix = 10;
                if (length == PRINTF_LENGTH_LONG_LONG)
                    printf_number(va_arg(args, unsigned long long), false, 0, radix);
                else if (length == PRINTF_LENGTH_LONG)
                    printf_number(va_arg(args, unsigned long), false, 0, radix);
                else
                    printf_number(va_arg(args, unsigned int), false, 0, radix);
                break;

            case 'x': case 'X': case 'p':
                sign = false; radix = 16;
                if (length == PRINTF_LENGTH_LONG_LONG)
                    printf_number(va_arg(args, unsigned long long), false, 0, radix);
                else if (length == PRINTF_LENGTH_LONG)
                    printf_number(va_arg(args, unsigned long), false, 0, radix);
                else
                    printf_number(va_arg(args, unsigned int), false, 0, radix);
                break;

            case 'o':
                sign = false; radix = 8;
                printf_number(va_arg(args, unsigned int), false, 0, radix);
                break;

            default: break;
            }
            state  = PRINTF_STATE_START;
            length = PRINTF_LENGTH_START;
            radix  = 10;
            sign   = false;
            break;
        }
        fmt++;
    }

    va_end(args);
}