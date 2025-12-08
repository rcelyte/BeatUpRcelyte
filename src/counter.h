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

static const uint8_t COUNTER64_INVALID = UINT8_MAX;
static const uint16_t COUNTERP_INVALID = UINT16_MAX;

static inline void Counter64_clear(struct Counter64 *const this, const uint32_t bit) {
	this->bits &= ~(UINT64_C(1) << bit);
}
static inline void Counter64_set(struct Counter64 *const this, const uint32_t bit) {
	this->bits |= UINT64_C(1) << bit;
}
static inline uint8_t Counter64_get_next(const struct Counter64 this) {
	return (this.bits != 0) ? (uint8_t)__builtin_ctzll(this.bits) : COUNTER64_INVALID;
}
static inline uint8_t Counter64_clear_next(struct Counter64 *const this) {
	if(this->bits == 0)
		return COUNTER64_INVALID;
	uint8_t bit = (uint8_t)__builtin_ctzll(this->bits);
	this->bits &= this->bits - 1u;
	return bit;
}
static inline uint8_t Counter64_set_next(struct Counter64 *const this) {
	if(~this->bits == 0)
		return COUNTER64_INVALID;
	uint8_t bit = (uint8_t)__builtin_ctzll(~this->bits);
	Counter64_set(this, bit);
	return bit;
}

static inline bool CounterP_get(struct CounterP this, const uint32_t bit) {
	return (this.bits[bit / 64] >> (bit % 64)) & 1;
}
static inline bool CounterP_clear(struct CounterP *const this, const uint32_t bit) {
	bool prev = CounterP_get(*this, bit);
	this->bits[bit / 64] &= ~(UINT64_C(1) << (bit % 64));
	return prev;
}
static inline bool CounterP_set(struct CounterP *const this, const uint32_t bit) {
	bool prev = CounterP_get(*this, bit);
	this->bits[bit / 64] |= UINT64_C(1) << (bit % 64);
	return prev;
}
static inline struct CounterP CounterP_single(uint32_t bit) {
	struct CounterP out = {};
	CounterP_set(&out, bit);
	return out;
}
static inline bool CounterP_overwrite(struct CounterP *const this, uint32_t bit, bool state) {
	return (state ? CounterP_set : CounterP_clear)(this, bit);
}
static inline uint16_t CounterP_get_next(const struct CounterP this, const bool value) {
	uint32_t word = 0;
	while((value ? this.bits[word] : ~this.bits[word]) == 0)
		if(++word >= lengthof(this.bits))
			return COUNTERP_INVALID;
	return word * 64 + __builtin_ctzll(value ? this.bits[word] : ~this.bits[word]);
}
[[maybe_unused]] static bool CounterP_clear_next(struct CounterP *const this, uint32_t *const out) {
	const uint16_t i = CounterP_get_next(*this, true);
	if(i == COUNTERP_INVALID)
		return false;
	this->bits[i / 64] &= this->bits[i / 64] - 1;
	*out = i;
	return true;
}
[[maybe_unused]] static bool CounterP_set_next(struct CounterP *const this, uint32_t *const out) {
	const uint16_t i = CounterP_get_next(*this, false);
	if(i == COUNTERP_INVALID)
		return false;
	this->bits[i / 64] |= this->bits[i / 64] + 1;
	*out |= i;
	return true;
}
static inline struct CounterP CounterP_and(const struct CounterP a, const struct CounterP b) {
	struct CounterP out = {0};
	for(uint32_t i = 0; i < lengthof(out.bits); ++i)
		out.bits[i] = a.bits[i] & b.bits[i];
	return out;
}
static inline struct CounterP CounterP_or(const struct CounterP a, const struct CounterP b) {
	struct CounterP out = {0};
	for(uint32_t i = 0; i < lengthof(out.bits); ++i)
		out.bits[i] = a.bits[i] | b.bits[i];
	return out;
}
static inline struct CounterP CounterP_xor(const struct CounterP a, const struct CounterP b) {
	struct CounterP out = {0};
	for(uint32_t i = 0; i < lengthof(out.bits); ++i)
		out.bits[i] = a.bits[i] ^ b.bits[i];
	return out;
}
static inline bool CounterP_isEmpty(const struct CounterP this) {
	return memcmp(this.bits, (struct CounterP){}.bits, sizeof(this.bits)) == 0;
}
static inline bool CounterP_contains(struct CounterP this, struct CounterP subset) {
	this = CounterP_and(this, subset);
	return memcmp(this.bits, subset.bits, sizeof(this.bits)) == 0;
}
static inline bool CounterP_containsNone(const struct CounterP this, struct CounterP subset) {
	return CounterP_isEmpty(CounterP_and(this, subset));
}
static inline uint8_t CounterP_byte(const struct CounterP this, uint32_t offset) {
	return (uint8_t)(this.bits[offset / 64] >> (offset % 64));
}
static inline uint32_t CounterP_count(const struct CounterP this) {
	uint32_t count = 0;
	for(const uint64_t *word = this.bits; word < endof(this.bits); ++word)
		count += (unsigned)__builtin_popcountll(*word);
	return count;
}
