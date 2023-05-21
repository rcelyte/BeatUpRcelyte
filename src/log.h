#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvla"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
static void vuprintf(const char *format, va_list vlist) {
	static bool carry = true;
	va_list args;
	va_copy(args, vlist);
	char buf[vsnprintf(NULL, 0, format, vlist) + 1]; // TODO: static buffer size; truncate message
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
	for(const char *end, *it = buf; it < &buf[sizeof(buf) - 1]; it = end) {
		#ifndef DISABLE_LOG_PREFIX
		if(carry)
			printf("[%s] ", __builtin_strrchr("/"__BASE_FILE__, '/') + 1);
		#endif
		end = strchr(it, '\n');
		carry = (end != NULL);
		if(carry)
			++end;
		else
			end = &buf[sizeof(buf) - 1];
		fwrite(it, 1, (size_t)(end - it), stdout);
		if(carry)
			fflush(stdout);
	}
}
#pragma GCC diagnostic pop

[[maybe_unused]] static void uprintf(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vuprintf(format, args);
	va_end(args);
}
