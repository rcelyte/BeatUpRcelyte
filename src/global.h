#pragma once
#include "log.h"
#include <stdint.h>
#define lengthof(x) (sizeof(x)/sizeof(*(x)))
#define endof(x) (&(x)[lengthof(x)])

[[maybe_unused]] static bool check_length(const char *errorMessage, const uint8_t *head, const uint8_t *end, size_t expect) {
	if(head == end)
		return false;
	const uint8_t *start = end - expect;
	uprintf("%s (expected %u, read %zu)\n\t", errorMessage, expect, head - start);
	for(const uint8_t *it = start; it < head; ++it)
		uprintf("%02hhx", *it);
	if(head - start < expect) {
		uprintf("\n\t");
		for(const uint8_t *it = start; it < head; ++it)
			uprintf("  ");
		uprintf("^ extra data starts here");
	}
	uprintf("\n");
	return true;
}
