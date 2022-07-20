#pragma once
#include "../global.h"

struct Counter16 {
	uint16_t bits;
};

struct Counter128 {
	uint64_t bits[2];
};

#define COUNTER16_CLEAR (struct Counter16){0};
#define COUNTER128_CLEAR (struct Counter128){{0, 0}};

static bool Counter16_set(struct Counter16 *set, uint8_t bit, bool state) {
	bool prev = (set->bits >> bit) & 1;
	if(state)
		set->bits |= 1 << bit;
	else
		set->bits &= ~(1 << bit);
	return prev;
}

static bool Counter128_get(struct Counter128 set, uint32_t bit) {
	return (set.bits[bit / 64] >> (bit % 64)) & 1;
}

static bool Counter128_set(struct Counter128 *set, uint32_t bit, bool state) {
	uint32_t i = bit / 64;
	bit %= 64;
	bool prev = (set->bits[i] >> bit) & 1;
	if(state)
		set->bits[i] |= 1 << bit;
	else
		set->bits[i] &= ~(1 << bit);
	return prev;
}

static bool Counter16_set_next(struct Counter16 *set, uint8_t *bit, bool state) {
	uint16_t v = state ? ~set->bits : set->bits;
	if(v == 0)
		return 0;
	*bit = __builtin_ctz(v);
	if(state)
		set->bits |= set->bits + 1;
	else
		set->bits &= set->bits - 1;
	return 1;
}

static bool Counter128_set_next_0(struct Counter128 *set, uint32_t *bit) {
	if(set->bits[0] | set->bits[1]) {
		uint8_t i = (set->bits[0] == 0);
		*bit = __builtin_ctzll(set->bits[i]) + (i * 64);
		set->bits[i] &= set->bits[i] - 1;
		return 1;
	}
	return 0;
}

static bool Counter128_set_next_1(struct Counter128 *set, uint32_t *bit) {
	if(~set->bits[0] | ~set->bits[1]) {
		uint8_t i = (set->bits[0] == ~0llu);
		*bit = __builtin_ctzll(~set->bits[i]) + (i * 64);
		set->bits[i] |= set->bits[i] + 1;
		return 1;
	}
	return 0;
}

#define Counter128_set_next(set, bit, state) Counter128_set_next_##state(set, bit)

static bool Counter128_eq(struct Counter128 a, struct Counter128 b) {
	for(uint32_t i = 0; i < lengthof(a.bits); ++i)
		if(a.bits[i] != b.bits[i])
			return 0;
	return 1;
}

static bool Counter128_contains(struct Counter128 set, struct Counter128 subset) {
	for(uint32_t i = 0; i < lengthof(set.bits); ++i)
		if((set.bits[i] & subset.bits[i]) != subset.bits[i])
			return 0;
	return 1;
}

static bool Counter128_containsNone(struct Counter128 set, struct Counter128 subset) {
	for(uint32_t i = 0; i < lengthof(set.bits); ++i)
		if(set.bits[i] & subset.bits[i])
			return 0;
	return 1;
}

static bool Counter128_isEmpty(struct Counter128 set) {
	for(uint32_t i = 0; i < lengthof(set.bits); ++i)
		if(set.bits[i])
			return 0;
	return 1;
}

static struct Counter128 Counter128_and(struct Counter128 a, struct Counter128 b) {
	for(uint32_t i = 0; i < lengthof(a.bits); ++i)
		a.bits[i] &= b.bits[i];
	return a;
}
