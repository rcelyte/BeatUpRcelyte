#include <stdio.h>
void LogString(const char*);

[[maybe_unused]] static void uprintf(const char *format, ...) {
	va_list args0, args1;
	va_start(args0, format);
	va_copy(args1, args0);
	char buf[vsnprintf(NULL, 0, format, args0) + 1];
	va_end(args0);
	vsnprintf(buf, sizeof(buf), format, args1);
	va_end(args1);
	LogString(buf);
}

#include "../../common/packets.c.h"
