#pragma once
#include "global.h"

struct Counter64 { // Typesafe wrapper for 64-bit bitfield operations
	uint64_t bits;
};

struct CounterP {
	#ifdef MP_EXTENDED_ROUTING
	uint64_t bits[4];
	#else
	uint64_t bits[2];
	#endif
};

static const struct Counter64 COUNTER64_CLEAR = {0};
static const struct CounterP COUNTERP_CLEAR = {0};
static const uint8_t COUNTER64_INVALID = 0xff;
static const uint16_t COUNTERP_INVALID = 0xffff;

static inline void Counter64_clear(struct Counter64 *set, uint32_t bit) {
	set->bits &= ~(UINT64_C(1) << bit);
}
static inline void Counter64_set(struct Counter64 *set, uint32_t bit) {
	set->bits |= UINT64_C(1) << bit;
}
static inline uint8_t Counter64_get_next(struct Counter64 set) {
	return (set.bits != 0) ? (uint8_t)__builtin_ctzll(set.bits) : COUNTER64_INVALID;
}
static inline uint8_t Counter64_clear_next(struct Counter64 *set) {
	if(set->bits == 0)
		return COUNTER64_INVALID;
	uint8_t bit = (uint8_t)__builtin_ctzll(set->bits);
	Counter64_clear(set, bit);
	return bit;
}
static inline uint8_t Counter64_set_next(struct Counter64 *set) {
	if(~set->bits == 0)
		return COUNTER64_INVALID;
	uint8_t bit = (uint8_t)__builtin_ctzll(~set->bits);
	Counter64_set(set, bit);
	return bit;
}
static inline bool CounterP_get(struct CounterP set, uint32_t bit) {
	return (set.bits[bit / 64] >> (bit % 64)) & 1;
}
static inline bool CounterP_clear(struct CounterP *set, uint32_t bit) {
	bool prev = CounterP_get(*set, bit);
	set->bits[bit / 64] &= ~(UINT64_C(1) << (bit % 64));
	return prev;
}
static inline bool CounterP_set(struct CounterP *set, uint32_t bit) {
	bool prev = CounterP_get(*set, bit);
	set->bits[bit / 64] |= UINT64_C(1) << (bit % 64);
	return prev;
}
static inline bool CounterP_overwrite(struct CounterP *set, uint32_t bit, bool state) {
	return (state ? CounterP_set : CounterP_clear)(set, bit);
}
[[maybe_unused]] static bool CounterP_clear_next(struct CounterP *set, uint32_t *out) {
	uint32_t i = 0;
	for(uint32_t word = 0; word < lengthof(set->bits); ++word)
		i |= (uint32_t)(set->bits[word] != UINT64_C(0)) << word;
	if(!i)
		return false;
	i = (uint32_t)__builtin_ctz(i);
	*out = (uint32_t)__builtin_ctzll(set->bits[i]);
	set->bits[i] &= ~(UINT64_C(1) << *out);
	*out |= i * 64;
	return true;
}
[[maybe_unused]] static bool CounterP_set_next(struct CounterP *set, uint32_t *out) {
	uint32_t i = 0;
	for(uint32_t word = 0; word < lengthof(set->bits); ++word)
		i |= (uint32_t)(~set->bits[word] != UINT64_C(0)) << word;
	if(!i)
		return false;
	i = (uint32_t)__builtin_ctz(i);
	*out = (uint32_t)__builtin_ctzll(~set->bits[i]);
	set->bits[i] |= UINT64_C(1) << *out;
	*out |= i * 64;
	return true;
}
static inline struct CounterP CounterP_and(struct CounterP a, struct CounterP b) {
	struct CounterP out = {0};
	for(uint32_t i = 0; i < lengthof(out.bits); ++i)
		out.bits[i] = a.bits[i] & b.bits[i];
	return out;
}
static inline struct CounterP CounterP_or(struct CounterP a, struct CounterP b) {
	struct CounterP out = {0};
	for(uint32_t i = 0; i < lengthof(out.bits); ++i)
		out.bits[i] = a.bits[i] | b.bits[i];
	return out;
}
static inline bool CounterP_isEmpty(struct CounterP set) {
	return memcmp(set.bits, COUNTERP_CLEAR.bits, sizeof(set.bits)) == 0;
}
static inline bool CounterP_contains(struct CounterP set, struct CounterP subset) {
	set = CounterP_and(set, subset);
	return memcmp(set.bits, subset.bits, sizeof(set.bits)) == 0;
}
static inline bool CounterP_containsNone(struct CounterP set, struct CounterP subset) {
	return CounterP_isEmpty(CounterP_and(set, subset));
}
static inline uint8_t CounterP_byte(struct CounterP set, uint32_t offset) {
	return (uint8_t)(set.bits[offset / 64] >> (offset % 64));
}
