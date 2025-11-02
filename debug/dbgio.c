#include <debug/dbgio.h>
#include <mdos/types.h>
#include <stdarg.h>
#include <stddef.h>

#define DBG_MAX_CHARSINKS 4
static dbg_charsink_t sinks[DBG_MAX_CHARSINKS];

mdos_result_t dbg_register_sink(dbg_charsink_t sink) {
    for (size_t i = 0; i < DBG_MAX_CHARSINKS; i++) {
        if (sinks[i] == NULL) {
            sinks[i] = sink;
            return MDOS_RES_SUCCESS;
        }
    }

    return MDOS_RES_OUT_OF_RESOURCES;
}

void dbg_unregister_sink(dbg_charsink_t sink) {
    for (size_t i = 0; i < DBG_MAX_CHARSINKS; i++) {
        if (sinks[i] == sink) {
            sinks[i] = NULL;
            return;
        }
    }
}

void dbg_sinkchr(char c) {
    for (size_t i = 0; i < DBG_MAX_CHARSINKS; i++) {
        if (sinks[i] != NULL) { sinks[i](c); }
    }
}

static inline __u64 dbg_strlen(const char *str) {
    __u64 len = 0;
    while (str[len] != 0) { len++; }
    return len;
}

char toU64StrBuff[128];
char toH64StrBuff[128];
char toH32StrBuff[128];
char toH16StrBuff[128];
char toH8StrBuff[128];
char toI64StrBuff[128];

static const char *u64_to_str(__u64 v) {
    __u64 sizeTest = v;
    __u8 length    = 0;
    while (sizeTest / 10 > 0) {
        sizeTest /= 10;
        length++;
    }

    __u8 index = 0;
    do {
        __u8 remainder = (__u8) (v % 10);
        v /= 10;
        toU64StrBuff[length - index] = (char) (remainder) + '0';
        index++;
    } while (v / 10 > 0);
    __u8 remainder               = (__u8) (v % 10);
    toU64StrBuff[length - index] = (char) (remainder) + '0';
    toU64StrBuff[length + 1]     = '\0';
    return toU64StrBuff;
}

static const char *u64_to_hstr(__u64 v) {
    static const char hexDigits[] = "0123456789ABCDEF";
    for (int i = 0; i < 16; ++i) { toH64StrBuff[15 - i] = hexDigits[(v >> (i * 4)) & 0xF]; }
    toH64StrBuff[16] = '\0';
    return toH64StrBuff;
}

static const char *u32_to_hstr(__u32 v) {
    static const char hexDigits[] = "0123456789ABCDEF";
    for (int i = 0; i < 8; ++i) { toH32StrBuff[7 - i] = hexDigits[(v >> (i * 4)) & 0xF]; }
    toH32StrBuff[8] = '\0';
    return toH32StrBuff;
}

// static const char *u16_to_hstr(__u16 v) {
// 	static const char hexDigits[] = "0123456789ABCDEF";
// 	for (int i = 0; i < 4; ++i) { toH16StrBuff[3 - i] = hexDigits[(v >> (i * 4)) & 0xF]; }
// 	toH16StrBuff[4] = '\0';
// 	return toH16StrBuff;
// }

// static const char *u8_to_hstr(__u8 v) {
// 	static const char hexDigits[] = "0123456789ABCDEF";
// 	for (int i = 0; i < 2; ++i) { toH8StrBuff[1 - i] = hexDigits[(v >> (i * 4)) & 0xF]; }
// 	toH8StrBuff[2] = '\0';
// 	return toH8StrBuff;
// }

static const char *s64_to_str(__s64 v) {
    __u8 negative = 0;
    if (v < 0) {
        negative = 1;
        v *= -1;
        toI64StrBuff[0] = '-';
    }
    __u64 sizeTest = (__u64) (v);
    __u8 length    = 0;
    while (sizeTest / 10 > 0) {
        sizeTest /= 10;
        length++;
    }

    __u8 index = 0;
    while (v / 10 > 0) {
        __u8 remainder = (__u8) (v % 10);
        v /= 10;
        toI64StrBuff[negative + length - index] = (char) (remainder) + '0';
        index++;
    }
    __u8 remainder                          = (__u8) (v % 10);
    toI64StrBuff[negative + length - index] = (char) (remainder) + '0';
    toI64StrBuff[negative + length + 1]     = '\0';
    return toI64StrBuff;
}

static void dbg_internal_print_str(const char *fmt, size_t len) {
    for (size_t i = 0; i < len; i++) { dbg_sinkchr(fmt[i]); }
}

static size_t dbg_internal_print(const char *fmt, va_list params) {
    size_t written = 0;
    while (*fmt != '\0') {
        size_t maxrem = SIZE_MAX - written;

        if (fmt[0] != '%' || fmt[1] == '%') {
            if (fmt[0] == '%') { fmt++; }
            size_t amount = 1;
            while (fmt[amount] && fmt[amount] != '%') { amount++; }
            if (maxrem < amount) {
                // TODO: error stuff
                return (size_t) 0;
            }
            dbg_internal_print_str(fmt, amount);
            fmt += amount;
            written += amount;
            continue;
        }

        const char *fmt_begun_at = fmt++;

        if (*fmt == 'c') {
            fmt++;
            char c = (char) va_arg(params, int);
            if (maxrem < 1) {
                // TODO: error
                return (size_t) 0;
            }
            dbg_internal_print_str(&c, 1);
            written++;
        } else if (*fmt == 's') {
            fmt++;
            const char *str = va_arg(params, const char *);
            size_t amount   = dbg_strlen(str);
            if (maxrem < amount) {
                // TODO: error
                return (size_t) 0;
            }
            dbg_internal_print_str(str, amount);
            written += amount;
        } else if (*fmt == 'd' || *fmt == 'i') {
            fmt++;
            __s32 num       = (__s32) va_arg(params, __s32);
            const char *str = s64_to_str((__s64) num);
            size_t amount   = dbg_strlen(str);
            if (maxrem < amount) {
                // TODO: error
                return (size_t) 0;
            }
            dbg_internal_print_str(str, amount);
            written += amount;
        } else if ((*fmt == 'l' && fmt[1] == 'd') || (*fmt == 'l' && fmt[1] == 'i')) {
            fmt += 2;
            __s64 num       = (__s64) va_arg(params, __s64);
            const char *str = s64_to_str((__s64) num);
            size_t amount   = dbg_strlen(str);
            if (maxrem < amount) {
                // TODO: error
                return (size_t) 0;
            }
            dbg_internal_print_str(str, amount);
            written += amount;
        } else if (*fmt == 'u') {
            fmt++;
            __u32 num       = (__u32) va_arg(params, __u32);
            const char *str = u64_to_str((__u64) num);
            size_t amount   = dbg_strlen(str);
            if (maxrem < amount) {
                // TODO: error
                return (size_t) 0;
            }
            dbg_internal_print_str(str, amount);
            written += amount;
        } else if ((*fmt == 'l' && fmt[1] == 'u')) {
            fmt += 2;
            __u64 num       = (__u64) va_arg(params, __u64);
            const char *str = u64_to_str((__u64) num);
            size_t amount   = dbg_strlen(str);
            if (maxrem < amount) {
                // TODO: error
                return (size_t) 0;
            }
            dbg_internal_print_str(str, amount);
            written += amount;
        } else if (*fmt == 'x') {
            fmt++;
            __u32 num       = (__u32) va_arg(params, __u32);
            const char *str = u32_to_hstr(num);
            size_t amount   = dbg_strlen(str);
            if (maxrem < amount) {
                // TODO: error
                return (size_t) 0;
            }
            dbg_internal_print_str(str, amount);
            written += amount;
        } else if ((*fmt == 'l' && fmt[1] == 'x')) {
            fmt += 2;
            __u64 num       = (__u64) va_arg(params, __u64);
            const char *str = u64_to_hstr(num);
            size_t amount   = dbg_strlen(str);
            if (maxrem < amount) {
                // TODO: error
                return (size_t) 0;
            }
            dbg_internal_print_str(str, amount);
            written += amount;
        } else {
            fmt           = fmt_begun_at;
            size_t amount = dbg_strlen(fmt);
            if (maxrem < amount) {
                // implement errno
                return (size_t) 0;
            }
            dbg_internal_print_str(fmt, amount);
            written += amount;
            fmt += amount;
        }
    }

    return written;
}

size_t dbg_msg(const char *fmt, ...) {
    va_list params;
    va_start(params, fmt);

    size_t written = dbg_internal_print(fmt, params);

    va_end(params);
    return written;
}