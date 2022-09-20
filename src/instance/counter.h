#pragma once
#include "../global.h"

struct Counter64 { // Typesafe wrapper for 64-bit bitfield operations
	uint64_t bits;
};

struct Counter128 {
	struct Counter64 sub[2];
};

static const struct Counter64 COUNTER64_CLEAR = {0};
static const struct Counter128 COUNTER128_CLEAR = {{{0}, {0}}};

static inline bool Counter64_get(struct Counter64 set, uint32_t bit) {
	return (set.bits >> bit) & 1;
}
static inline bool Counter64_clear(struct Counter64 *set, uint32_t bit) {
	bool prev = Counter64_get(*set, bit);
	set->bits &= ~(1 << bit);
	return prev;
}
static inline bool Counter64_set(struct Counter64 *set, uint32_t bit) {
	bool prev = Counter64_get(*set, bit);
	set->bits |= 1 << bit;
	return prev;
}
static inline bool Counter64_overwrite(struct Counter64 *set, uint32_t bit, bool state) {
	return (state ? Counter64_set : Counter64_clear)(set, bit);
}
static inline bool Counter64_clear_next(struct Counter64 *set, uint32_t *bit) {
	if(set->bits == 0)
		return false;
	*bit = __builtin_ctzll(set->bits);
	Counter64_clear(set, *bit);
	return true;
}
static inline bool Counter64_set_next(struct Counter64 *set, uint32_t *bit) {
	if(~set->bits == 0)
		return false;
	*bit = __builtin_ctzll(~set->bits);
	Counter64_set(set, *bit);
	return true;
}
static inline bool Counter64_eq(struct Counter64 a, struct Counter64 b) {
	return a.bits == b.bits;
}
static inline bool Counter64_isEmpty(struct Counter64 set) {
	return set.bits == 0;
}
static inline bool Counter64_isFilled(struct Counter64 set) {
	return ~set.bits == 0;
}
static inline struct Counter64 Counter64_and(struct Counter64 a, struct Counter64 b) {
	return (struct Counter64){a.bits & b.bits};
}
static inline struct Counter64 Counter64_or(struct Counter64 a, struct Counter64 b) {
	return (struct Counter64){a.bits | b.bits};
}
static inline bool Counter64_contains(struct Counter64 set, struct Counter64 subset) {
	return (set.bits & subset.bits) == subset.bits;
}
static inline bool Counter64_containsNone(struct Counter64 set, struct Counter64 subset) {
	return (set.bits & subset.bits) == 0;
}

static inline bool Counter128_get(struct Counter128 set, uint32_t bit) {
	return Counter64_get(set.sub[bit / 64], bit % 64);
}
static inline bool Counter128_clear(struct Counter128 *set, uint32_t bit) {
	return Counter64_clear(&set->sub[bit / 64], bit % 64);
}
static inline bool Counter128_set(struct Counter128 *set, uint32_t bit) {
	return Counter64_set(&set->sub[bit / 64], bit % 64);
}
static inline bool Counter128_overwrite(struct Counter128 *set, uint32_t bit, bool state) {
	return (state ? Counter128_set : Counter128_clear)(set, bit);
}
[[maybe_unused]] static bool Counter128_clear_next(struct Counter128 *set, uint32_t *bit) {
	uint8_t i = Counter64_isEmpty(set->sub[0]);
	if(!Counter64_clear_next(&set->sub[i], bit))
		return false;
	*bit += i * 64;
	return true;
}
[[maybe_unused]] static bool Counter128_set_next(struct Counter128 *set, uint32_t *bit) {
	uint8_t i = Counter64_isFilled(set->sub[0]);
	if(!Counter64_set_next(&set->sub[i], bit))
		return false;
	*bit += i * 64;
	return true;
}
static inline bool Counter128_eq(struct Counter128 a, struct Counter128 b) {
	return Counter64_eq(a.sub[0], b.sub[0]) && Counter64_eq(a.sub[1], b.sub[1]);
}
static inline bool Counter128_contains(struct Counter128 set, struct Counter128 subset) {
	return Counter64_contains(set.sub[0], subset.sub[0]) && Counter64_contains(set.sub[1], subset.sub[1]);
}
static inline bool Counter128_containsNone(struct Counter128 set, struct Counter128 subset) {
	return Counter64_containsNone(set.sub[0], subset.sub[0]) && Counter64_containsNone(set.sub[1], subset.sub[1]);
}
static inline bool Counter128_isEmpty(struct Counter128 set) {
	return Counter64_isEmpty(set.sub[0]) && Counter64_isEmpty(set.sub[1]);
}
static inline struct Counter128 Counter128_and(struct Counter128 a, struct Counter128 b) {
	return (struct Counter128){{
		Counter64_and(a.sub[0], b.sub[0]),
		Counter64_and(a.sub[1], b.sub[1]),
	}};
}
static inline struct Counter128 Counter128_or(struct Counter128 a, struct Counter128 b) {
	return (struct Counter128){{
		Counter64_or(a.sub[0], b.sub[0]),
		Counter64_or(a.sub[1], b.sub[1]),
	}};
}
