#include <string.h>
#include <setjmp.h>
#include <assert.h>

size_t pkt_write_bytes(const uint8_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext, size_t count) {
	if(end - *pkt < (ptrdiff_t)count)
		return 0;
	memcpy(*pkt, data, count);
	*pkt += count;
	return count;
}

// MinGW is missing `threads.h`
static _Thread_local jmp_buf fail;
size_t _pkt_try_read(void (*inner)(void *restrict, const uint8_t**, const uint8_t*, struct PacketContext), void *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	const uint8_t *start = *pkt;
	if(setjmp(fail)) {
		*pkt = start;
		return 0;
	}
	inner(data, pkt, end, ctx);
	return (size_t)(*pkt - start);
}
size_t _pkt_try_write(void (*inner)(const void *restrict, uint8_t**, const uint8_t*, struct PacketContext), const void *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint8_t *start = *pkt;
	if(setjmp(fail)) {
		*pkt = start;
		return 0;
	}
	inner(data, pkt, end, ctx);
	return (size_t)(*pkt - start);
}
[[gnu::noreturn]] static void range_fail() {
	uprintf("Unexpected end of packet\n");
	longjmp(fail, 1);
}
#define RANGE_CHECK(size) \
	if(end - *pkt < (ptrdiff_t)(size)) \
		range_fail()
static uint32_t check_overflow(uint32_t count, uint32_t limit, const char *context) {
	if(count <= limit)
		return count;
	uprintf("Buffer overflow in read of `%s`\n", context);
	longjmp(fail, 1);
}
static_assert(sizeof(uint8_t) == 1, "");
#define _pkt_b_read(data, pkt, end, ctx) _pkt_u8_read((uint8_t*)(data), pkt, end, ctx)
#define _pkt_i8_read(data, pkt, end, ctx) _pkt_u8_read((uint8_t*)(data), pkt, end, ctx)
static void _pkt_u8_read(uint8_t *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext) {
	RANGE_CHECK(sizeof(*data));
	*data = *(*pkt)++;
}
static_assert(sizeof(uint16_t) == 2, "");
#define _pkt_i16_read(data, pkt, end, ctx) _pkt_u16_read((uint16_t*)(data), pkt, end, ctx)
[[maybe_unused]] static void _pkt_u16_read(uint16_t *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext) {
	RANGE_CHECK(sizeof(*data));
	*data = (*pkt)[0] | (uint16_t)((*pkt)[1] << 8);
	*pkt += sizeof(*data);
}
static_assert(sizeof(uint32_t) == 4, "");
#define _pkt_i32_read(data, pkt, end, ctx) _pkt_u32_read((uint32_t*)(data), pkt, end, ctx)
[[maybe_unused]] static void _pkt_u32_read(uint32_t *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext) {
	RANGE_CHECK(sizeof(*data));
	*data = (*pkt)[0] | (uint32_t)(*pkt)[1] << 8 | (uint32_t)(*pkt)[2] << 16 | (uint32_t)(*pkt)[3] << 24;
	*pkt += sizeof(*data);
}
static_assert(sizeof(uint64_t) == 8, "");
#define _pkt_i64_read(data, pkt, end, ctx) _pkt_u64_read((uint64_t*)(data), pkt, end, ctx)
[[maybe_unused]] static void _pkt_u64_read(uint64_t *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext) {
	RANGE_CHECK(sizeof(*data));
	*data = (uint64_t)(*pkt)[0] | (uint64_t)(*pkt)[1] << 8 | (uint64_t)(*pkt)[2] << 16 | (uint64_t)(*pkt)[3] << 24 | (uint64_t)(*pkt)[4] << 32 | (uint64_t)(*pkt)[5] << 40 | (uint64_t)(*pkt)[6] << 48 | (uint64_t)(*pkt)[7] << 56;
	*pkt += sizeof(*data);
}
static void _pkt_vu64_read(uint64_t *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	*data = 0;
	uint8_t byte, shift = 0;
	do {
		check_overflow(shift, 63, "vu64");
		_pkt_u8_read(&byte, pkt, end, ctx);
		*data |= (byte & 127llu) << shift;
		shift += 7;
	} while(byte & 128);
}
static void _pkt_vi64_read(int64_t *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu64_read((uint64_t*)data, pkt, end, ctx);
	bool negative = ((*data & 1ll) == 1ll);
	*data >>= 1;
	if(negative)
		*data = 1ll - *data;
}
[[maybe_unused]] static void _pkt_vu32_read(uint32_t *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint64_t tmp;
	_pkt_vu64_read(&tmp, pkt, end, ctx);
	*data = (uint32_t)tmp;
}
[[maybe_unused]] static void _pkt_vi32_read(int32_t *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	int64_t tmp;
	_pkt_vi64_read(&tmp, pkt, end, ctx);
	*data = (int32_t)tmp;
}
[[maybe_unused]] static void _pkt_raw_read(uint8_t *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext, size_t count) {
	RANGE_CHECK(count);
	memcpy(data, *pkt, count);
	*pkt += count;
}
static_assert(sizeof(float) == 4, "");
static_assert(sizeof(double) == 8, "");
#define _pkt_f32_read(data, pkt, end, ctx) _pkt_raw_read((uint8_t*)(data), pkt, end, ctx, sizeof(float))
#define _pkt_f64_read(data, pkt, end, ctx) _pkt_raw_read((uint8_t*)(data), pkt, end, ctx, sizeof(double))

#define _pkt_b_write(data, pkt, end, ctx) _pkt_u8_write((const uint8_t*)(data), pkt, end, ctx)
#define _pkt_i8_write(data, pkt, end, ctx) _pkt_u8_write((const uint8_t*)(data), pkt, end, ctx)
static void _pkt_u8_write(const uint8_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext) {
	RANGE_CHECK(sizeof(*data));
	*(*pkt)++ = *data;
}
#define _pkt_i16_write(data, pkt, end, ctx) _pkt_u16_write((const uint16_t*)(data), pkt, end, ctx)
[[maybe_unused]] static void _pkt_u16_write(const uint16_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext) {
	RANGE_CHECK(sizeof(*data));
	*(*pkt)++ = *data & 255;
	*(*pkt)++ = *data >> 8 & 255;
}
#define _pkt_i32_write(data, pkt, end, ctx) _pkt_u32_write((const uint32_t*)(data), pkt, end, ctx)
[[maybe_unused]] static void _pkt_u32_write(const uint32_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext) {
	RANGE_CHECK(sizeof(*data));
	*(*pkt)++ = *data & 255;
	*(*pkt)++ = *data >> 8 & 255;
	*(*pkt)++ = *data >> 16 & 255;
	*(*pkt)++ = *data >> 24 & 255;
}
#define _pkt_i64_write(data, pkt, end, ctx) _pkt_u64_write((const uint64_t*)(data), pkt, end, ctx)
[[maybe_unused]] static void _pkt_u64_write(const uint64_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext) {
	RANGE_CHECK(sizeof(*data));
	*(*pkt)++ = *data & 255;
	*(*pkt)++ = *data >> 8 & 255;
	*(*pkt)++ = *data >> 16 & 255;
	*(*pkt)++ = *data >> 24 & 255;
	*(*pkt)++ = *data >> 32 & 255;
	*(*pkt)++ = *data >> 40 & 255;
	*(*pkt)++ = *data >> 48 & 255;
	*(*pkt)++ = *data >> 56 & 255;
}
static void _pkt_vu64_write(const uint64_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint64_t v = *data;
	for(; v >= 128; v >>= 7)
		_pkt_u8_write((uint8_t[]){(uint8_t)v | 128u}, pkt, end, ctx);
	_pkt_u8_write((uint8_t[]){(uint8_t)v}, pkt, end, ctx);
}
static void _pkt_vi64_write(const int64_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint64_t v = (*data < 0) ? (uint64_t)((-(*data + 1) << 1) + 1) : (uint64_t)*data << 1;
	_pkt_vu64_write(&v, pkt, end, ctx);
}
[[maybe_unused]] static void _pkt_vu32_write(const uint32_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu64_write((uint64_t[]){*data}, pkt, end, ctx);
}
[[maybe_unused]] static void _pkt_vi32_write(const int32_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi64_write((int64_t[]){*data}, pkt, end, ctx);
}
[[maybe_unused]] static inline void _pkt_raw_write(const uint8_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext, size_t count) {
	RANGE_CHECK(count);
	memcpy(*pkt, data, count);
	*pkt += count;
}
#define _pkt_f32_write(data, pkt, end, ctx) _pkt_raw_write((const uint8_t*)(data), pkt, end, ctx, sizeof(float))
#define _pkt_f64_write(data, pkt, end, ctx) _pkt_raw_write((const uint8_t*)(data), pkt, end, ctx, sizeof(double))

[[maybe_unused]] static void _pkt_Cookie32_read(struct Cookie32 *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_raw_read(data->raw, pkt, end, ctx, 32);
}
[[maybe_unused]] static void _pkt_Cookie32_write(const struct Cookie32 *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_raw_write(data->raw, pkt, end, ctx, 32);
}

static uint32_t _pkt_BaseString_read(const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	if(ctx.netVersion < 12) {
		uint32_t length32;
		_pkt_u32_read(&length32, pkt, end, ctx);
		return length32 + 1;
	}
	uint16_t length16;
	_pkt_u16_read(&length16, pkt, end, ctx);
	return length16;
}

#define STRING_RDWR_FUNC(type) \
	[[maybe_unused]] static void _pkt_##type##_read(struct type *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) { \
		data->length = _pkt_BaseString_read(pkt, end, ctx); \
		data->isNull = (data->length == 0); \
		data->length -= !data->isNull; \
		check_overflow(data->length, sizeof(data->data), #type ".data"); \
		_pkt_raw_read((uint8_t*)data->data, pkt, end, ctx, data->length); \
	} \
	[[maybe_unused]] static void _pkt_##type##_write(const struct type *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) { \
		if(ctx.netVersion < 12) \
			_pkt_u32_write(&data->length, pkt, end, ctx); \
		else \
			_pkt_u16_write((uint16_t[]){(uint16_t)data->length + !data->isNull}, pkt, end, ctx); \
		_pkt_raw_write((const uint8_t*)data->data, pkt, end, ctx, data->length); \
	}
STRING_RDWR_FUNC(String)
STRING_RDWR_FUNC(LongString)
