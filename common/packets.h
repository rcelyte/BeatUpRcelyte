#pragma once
#include <stdint.h>
#include <stdbool.h>

struct String {
	uint32_t length;
	bool isNull;
	char data[60];
};
struct LongString {
	uint32_t length;
	bool isNull;
	char data[4096];
};

#define String_eq(a, b) ((a).length == (b).length && memcmp((a).data, (b).data, (b).length) == 0)
#define String_is(a, str) ((a).length == (lengthof(str) - 1) && memcmp((a).data, str, (lengthof(str) - 1)) == 0)
#ifdef __cplusplus
#define String_from(str) {lengthof(str) - 1, false, str}
#else
#define LongString_from(str) (struct LongString){lengthof(str) - 1, false, str}
#define String_from(str) (struct String){lengthof(str) - 1, false, str}
#endif
