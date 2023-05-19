#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

struct PacketContext {
	uint8_t netVersion;
	uint8_t protocolVersion;
	uint8_t beatUpVersion;
	bool direct;
	uint16_t windowSize;
};
struct String {
	uint32_t length;
	bool isNull;
	char data[67];
};
struct LongString {
	uint32_t length;
	bool isNull;
	char data[4091];
};
struct Cookie32 {
	uint8_t raw[32];
};

static inline bool String_eq(struct String a, struct String b) {return a.length == b.length && memcmp(a.data, b.data, b.length) == 0;}
static inline bool LongString_eq(struct LongString a, struct LongString b) {return a.length == b.length && memcmp(a.data, b.data, b.length) == 0;}

#ifdef __cplusplus
#define String_from(str) {lengthof(str) - 1, false, str}
#else
#define LongString_from(str) (struct LongString){lengthof(str) - 1, false, str}
#define String_from(str) (struct String){lengthof(str) - 1, false, str}
#endif
#define String_is(a, str) String_eq(a, String_from(str))

[[gnu::format(printf, 1, 2)]] static inline struct String String_fmt(const char *format, ...) {
	va_list args;
	va_start(args, format);
	struct String out = {.isNull = false};
	out.length = (uint32_t)vsnprintf(out.data, sizeof(out.data) / sizeof(*out.data), format, args);
	va_end(args);
	return out;
}

[[gnu::format(printf, 1, 2)]] static inline struct LongString LongString_fmt(const char *format, ...) {
	va_list args;
	va_start(args, format);
	struct LongString out = {.isNull = false};
	out.length = (uint32_t)vsnprintf(out.data, sizeof(out.data) / sizeof(*out.data), format, args);
	va_end(args);
	return out;
}
