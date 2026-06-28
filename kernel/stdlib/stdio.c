#include "stdio.h"

typedef enum {
    PRINTF_STATE_START,
    PRINTF_STATE_FLAGS,
    PRINTF_STATE_WIDTH,
    PRINTF_STATE_LENGTH,
    PRINTF_STATE_LENGTH_L,   /* seen one 'l' */
    PRINTF_STATE_LENGTH_H,   /* seen one 'h' */
    PRINTF_STATE_SPEC,
} printf_state;

typedef enum {
    PRINTF_LEN_DEFAULT,
    PRINTF_LEN_HH,           /* %hh  – char          */
    PRINTF_LEN_H,            /* %h   – short         */
    PRINTF_LEN_L,            /* %l   – long          */
    PRINTF_LEN_LL,           /* %ll  – long long     */
} printf_length;

/* ── printf_number ──────────────────────────────────────────────────────────
 *
 *  number     : value to print (always passed as uint64_t)
 *  is_signed  : true if the specifier is signed (%d/%i)
 *  signed_val : the signed interpretation (used only when is_signed=true)
 *  radix      : 10 or 16
 *  width      : minimum field width (0 = none)
 *  zero_pad   : pad with '0' instead of ' '
 */
static const char hex_digits[] = "0123456789abcdef";

static void printf_number(uint64_t number,
                           bool     is_signed,
                           int64_t  signed_val,
                           int      radix,
                           int      width,
                           bool     zero_pad)
{
    char buf[64];
    int  pos      = 0;
    bool negative = false;

    if (is_signed && signed_val < 0) {
        negative = true;
        number   = (uint64_t)(-signed_val);
    }

    /* Build digits in reverse */
    do {
        buf[pos++] = hex_digits[number % radix];
        number    /= radix;
    } while (number > 0);

    if (negative) buf[pos++] = '-';

    /* Pad to requested width */
    char pad_char = zero_pad ? '0' : ' ';
    while (pos < width) buf[pos++] = pad_char;

    /* Print in correct order */
    while (--pos >= 0) putchar(buf[pos]);
}

static void vprintf_core(const char *fmt, va_list args) {
    printf_state  state    = PRINTF_STATE_START;
    printf_length length   = PRINTF_LEN_DEFAULT;
    int           width    = 0;
    bool          zero_pad = false;

    while (*fmt) {
        switch (state) {

        /* ── Normal text ── */
        case PRINTF_STATE_START:
            if (*fmt == '%') {
                state    = PRINTF_STATE_FLAGS;
                width    = 0;
                zero_pad = false;
                length   = PRINTF_LEN_DEFAULT;
            } else {
                putchar(*fmt);
            }
            break;

        /* ── Flags: currently handles '0' for zero-padding ── */
        case PRINTF_STATE_FLAGS:
            if (*fmt == '0') {
                zero_pad = true;
                break;          /* stay in FLAGS, consume char */
            }
            state = PRINTF_STATE_WIDTH;
            /* fall through — reprocess this char as width */
            /* FALLTHROUGH */

        /* ── Width: decimal digits ── */
        case PRINTF_STATE_WIDTH:
            if (*fmt >= '1' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                break;
            }
            state = PRINTF_STATE_LENGTH;
            /* fall through — reprocess this char as length/spec */
            /* FALLTHROUGH */

        /* ── Length modifiers ── */
        case PRINTF_STATE_LENGTH:
            if (*fmt == 'l') { state = PRINTF_STATE_LENGTH_L; break; }
            if (*fmt == 'h') { state = PRINTF_STATE_LENGTH_H; break; }
            goto SPEC;

        case PRINTF_STATE_LENGTH_L:
            if (*fmt == 'l') { length = PRINTF_LEN_LL; state = PRINTF_STATE_SPEC; break; }
            length = PRINTF_LEN_L;
            goto SPEC;

        case PRINTF_STATE_LENGTH_H:
            if (*fmt == 'h') { length = PRINTF_LEN_HH; state = PRINTF_STATE_SPEC; break; }
            length = PRINTF_LEN_H;
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

            /* Signed decimal */
            case 'd': case 'i': {
                int64_t  sv;
                uint64_t uv;
                if (length == PRINTF_LEN_LL)      { sv = (int64_t)va_arg(args, long long); }
                else if (length == PRINTF_LEN_L)  { sv = (int64_t)va_arg(args, long);      }
                else if (length == PRINTF_LEN_H)  { sv = (int16_t)va_arg(args, int);       }
                else if (length == PRINTF_LEN_HH) { sv = (int8_t) va_arg(args, int);       }
                else                              { sv = (int64_t)va_arg(args, int);        }
                uv = (sv < 0) ? (uint64_t)(-sv) : (uint64_t)sv;
                printf_number(uv, true, sv, 10, width, zero_pad);
                break;
            }

            /* Unsigned decimal */
            case 'u': {
                uint64_t uv;
                if (length == PRINTF_LEN_LL)      uv = (uint64_t)va_arg(args, unsigned long long);
                else if (length == PRINTF_LEN_L)  uv = (uint64_t)va_arg(args, unsigned long);
                else if (length == PRINTF_LEN_H)  uv = (uint16_t)va_arg(args, unsigned int);
                else if (length == PRINTF_LEN_HH) uv = (uint8_t) va_arg(args, unsigned int);
                else                              uv = (uint64_t)va_arg(args, unsigned int);
                printf_number(uv, false, 0, 10, width, zero_pad);
                break;
            }

            /* Hex / pointer */
            case 'x': case 'X': case 'p': {
                uint64_t uv;
                if (*fmt == 'p' || length == PRINTF_LEN_LL)
                                                  uv = (uint64_t)va_arg(args, unsigned long long);
                else if (length == PRINTF_LEN_L)  uv = (uint64_t)va_arg(args, unsigned long);
                else if (length == PRINTF_LEN_H)  uv = (uint16_t)va_arg(args, unsigned int);
                else if (length == PRINTF_LEN_HH) uv = (uint8_t) va_arg(args, unsigned int);
                else                              uv = (uint64_t)va_arg(args, unsigned int);
                printf_number(uv, false, 0, 16, width, zero_pad);
                break;
            }

            /* Octal */
            case 'o': {
                uint64_t uv = (uint64_t)va_arg(args, unsigned int);
                printf_number(uv, false, 0, 8, width, zero_pad);
                break;
            }

            default:
                putchar('?');   /* unknown specifier — print a marker */
                break;
            }

            /* Reset for next specifier */
            state    = PRINTF_STATE_START;
            length   = PRINTF_LEN_DEFAULT;
            width    = 0;
            zero_pad = false;
            break;
        }

        fmt++;
    }
}

void vprintf(const char *fmt, va_list args) {
    vprintf_core(fmt, args);
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf_core(fmt, args);
    va_end(args);
}