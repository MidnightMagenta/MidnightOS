#include <debug/dbgio.h>
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

static inline uint64_t dbg_strlen(const char *str) {
	uint64_t len = 0;
	while (str[len] != 0) { len++; }
	return len;
}

char toU64StrBuff[128];
char toH64StrBuff[128];
char toH32StrBuff[128];
char toH16StrBuff[128];
char toH8StrBuff[128];
char toI64StrBuff[128];

static const char *u64_to_str(uint64_t v) {
	uint64_t sizeTest = v;
	uint8_t length = 0;
	while (sizeTest / 10 > 0) {
		sizeTest /= 10;
		length++;
	}

	uint8_t index = 0;
	do {
		uint8_t remainder = (uint8_t) (v % 10);
		v /= 10;
		toU64StrBuff[length - index] = (char) (remainder) + '0';
		index++;
	} while (v / 10 > 0);
	uint8_t remainder = (uint8_t) (v % 10);
	toU64StrBuff[length - index] = (char) (remainder) + '0';
	toU64StrBuff[length + 1] = '\0';
	return toU64StrBuff;
}

static const char *u64_to_hstr(uint64_t v) {
	static const char hexDigits[] = "0123456789ABCDEF";
	for (int i = 0; i < 16; ++i) { toH64StrBuff[15 - i] = hexDigits[(v >> (i * 4)) & 0xF]; }
	toH64StrBuff[16] = '\0';
	return toH64StrBuff;
}

static const char *u32_to_hstr(uint32_t v) {
	static const char hexDigits[] = "0123456789ABCDEF";
	for (int i = 0; i < 8; ++i) { toH32StrBuff[7 - i] = hexDigits[(v >> (i * 4)) & 0xF]; }
	toH32StrBuff[8] = '\0';
	return toH32StrBuff;
}

// static const char *u16_to_hstr(uint16_t v) {
// 	static const char hexDigits[] = "0123456789ABCDEF";
// 	for (int i = 0; i < 4; ++i) { toH16StrBuff[3 - i] = hexDigits[(v >> (i * 4)) & 0xF]; }
// 	toH16StrBuff[4] = '\0';
// 	return toH16StrBuff;
// }

// static const char *u8_to_hstr(uint8_t v) {
// 	static const char hexDigits[] = "0123456789ABCDEF";
// 	for (int i = 0; i < 2; ++i) { toH8StrBuff[1 - i] = hexDigits[(v >> (i * 4)) & 0xF]; }
// 	toH8StrBuff[2] = '\0';
// 	return toH8StrBuff;
// }

static const char *s64_to_str(int64_t v) {
	uint8_t negative = 0;
	if (v < 0) {
		negative = 1;
		v *= -1;
		toI64StrBuff[0] = '-';
	}
	uint64_t sizeTest = (uint64_t) (v);
	uint8_t length = 0;
	while (sizeTest / 10 > 0) {
		sizeTest /= 10;
		length++;
	}

	uint8_t index = 0;
	while (v / 10 > 0) {
		uint8_t remainder = (uint8_t) (v % 10);
		v /= 10;
		toI64StrBuff[negative + length - index] = (char) (remainder) + '0';
		index++;
	}
	uint8_t remainder = (uint8_t) (v % 10);
	toI64StrBuff[negative + length - index] = (char) (remainder) + '0';
	toI64StrBuff[negative + length + 1] = '\0';
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
			size_t ammount = 1;
			while (fmt[ammount] && fmt[ammount] != '%') { ammount++; }
			if (maxrem < ammount) {
				//TODO: error stuff
				return (size_t) 0;
			}
			dbg_internal_print_str(fmt, ammount);
			fmt += ammount;
			written += ammount;
			continue;
		}

		const char *fmt_begun_at = fmt++;

		if (*fmt == 'c') {
			fmt++;
			char c = (char) va_arg(params, int);
			if (maxrem < 1) {
				//TODO: error
				return (size_t) 0;
			}
			dbg_internal_print_str(&c, 1);
			written++;
		} else if (*fmt == 's') {
			fmt++;
			const char *str = va_arg(params, const char *);
			size_t ammount = dbg_strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return (size_t) 0;
			}
			dbg_internal_print_str(str, ammount);
			written += ammount;
		} else if (*fmt == 'd' || *fmt == 'i') {
			fmt++;
			int32_t num = (int32_t) va_arg(params, int32_t);
			const char *str = s64_to_str((int64_t) num);
			size_t ammount = dbg_strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return (size_t) 0;
			}
			dbg_internal_print_str(str, ammount);
			written += ammount;
		} else if ((*fmt == 'l' && fmt[1] == 'd') || (*fmt == 'l' && fmt[1] == 'i')) {
			fmt += 2;
			int64_t num = (int64_t) va_arg(params, int64_t);
			const char *str = s64_to_str((int64_t) num);
			size_t ammount = dbg_strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return (size_t) 0;
			}
			dbg_internal_print_str(str, ammount);
			written += ammount;
		} else if (*fmt == 'u') {
			fmt++;
			uint32_t num = (uint32_t) va_arg(params, uint32_t);
			const char *str = u64_to_str((uint64_t) num);
			size_t ammount = dbg_strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return (size_t) 0;
			}
			dbg_internal_print_str(str, ammount);
			written += ammount;
		} else if ((*fmt == 'l' && fmt[1] == 'u')) {
			fmt += 2;
			uint64_t num = (uint64_t) va_arg(params, uint64_t);
			const char *str = u64_to_str((uint64_t) num);
			size_t ammount = dbg_strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return (size_t) 0;
			}
			dbg_internal_print_str(str, ammount);
			written += ammount;
		} else if (*fmt == 'x') {
			fmt++;
			uint32_t num = (uint32_t) va_arg(params, uint32_t);
			const char *str = u32_to_hstr(num);
			size_t ammount = dbg_strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return (size_t) 0;
			}
			dbg_internal_print_str(str, ammount);
			written += ammount;
		} else if ((*fmt == 'l' && fmt[1] == 'x')) {
			fmt += 2;
			uint64_t num = (uint64_t) va_arg(params, uint64_t);
			const char *str = u64_to_hstr(num);
			size_t ammount = dbg_strlen(str);
			if (maxrem < ammount) {
				//TODO: error
				return (size_t) 0;
			}
			dbg_internal_print_str(str, ammount);
			written += ammount;
		} else {
			fmt = fmt_begun_at;
			size_t len = dbg_strlen(fmt);
			if (maxrem < len) {
				//implement errno
				return (size_t) 0;
			}
			dbg_internal_print_str(fmt, len);
			written += len;
			fmt += len;
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