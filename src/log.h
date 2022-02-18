#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define LOG_CTX(name) static void uprintf(const char *format, ...) { \
	static _Bool carry = 1; \
	va_list args0, args1; \
	va_start(args0, format); \
	va_copy(args1, args0); \
	char buf[vsnprintf(NULL, 0, format, args0) + 1]; \
	vsnprintf(buf, sizeof(buf), format, args1); \
	va_end(args0); \
	va_end(args1); \
	for(const char *end, *it = buf; it < &buf[sizeof(buf) - 1]; it = end) { \
		if(carry) \
			printf("[" name "] "); \
		end = strchr(it, '\n'); \
		if(end) \
			++end, carry = 1; \
		else \
			end = &buf[sizeof(buf) - 1], carry = 0; \
		fwrite(it, 1, end - it, stdout); \
		if(carry) \
			fflush(stdout); \
	} \
}
