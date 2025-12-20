#include <string.h>
#include <setjmp.h>
#include <assert.h>

static thread_local struct TraceInfo {
	jmp_buf fail;
	const uint8_t *start;
	const char *stack[32];
} *trace = NULL;


#define SCOPE_FUNC(_name, _type) static inline _type _name(_type state, const char name[const]) { \
	const unsigned depth = state.traceDepth++; \
	if(depth < lengthof(trace->stack)) \
		trace->stack[depth] = name; \
	return state; \
}
SCOPE_FUNC(PacketRead_scope, struct PacketRead)
SCOPE_FUNC(PacketWrite_scope, struct PacketWrite)
#undef SCOPE_FUNC
#define scope(_parent, _name) _Generic((_parent), struct PacketRead: PacketRead_scope, struct PacketWrite: PacketWrite_scope)(_parent, _name)

static inline struct PacketRead PacketRead_toRead(const struct PacketRead state) {
	return state;
}
static inline struct PacketRead PacketWrite_toRead(const struct PacketWrite state) {
	return (struct PacketRead){
		.head = (const uint8_t**)state.head,
		.end = state.end,
		.context = state.context,
		.traceDepth = state.traceDepth,
	};
}

size_t pkt_write_bytes(const uint8_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext, size_t count) {
	if(end - *pkt < (ptrdiff_t)count)
		return 0;
	memcpy(*pkt, data, count);
	*pkt += count;
	return count;
}
size_t _pkt_try_read(PacketReadFunc *const inner, void *restrict data, const uint8_t **pkt, const uint8_t *end, const struct PacketContext ctx) {
	struct TraceInfo state = {.start = *pkt};
	trace = &state;
	if(setjmp(state.fail)) {
		*pkt = state.start;
		trace = NULL;
		return 0;
	}
	inner(data, (struct PacketRead){
		.head = pkt,
		.end = end,
		.context = ctx,
	});
	trace = NULL;
	return (size_t)(*pkt - state.start);
}
size_t _pkt_try_write(PacketWriteFunc *const inner, const void *restrict data, uint8_t **pkt, const uint8_t *end, const struct PacketContext ctx) {
	struct TraceInfo state = {.start = *pkt};
	trace = &state;
	if(setjmp(state.fail)) {
		*pkt -= *pkt - state.start;
		trace = NULL;
		return 0;
	}
	inner(data, (struct PacketWrite){
		.head = pkt,
		.end = end,
		.context = ctx,
	});
	trace = NULL;
	return (size_t)(*pkt - state.start);
}
void pkt_dump(const char *errorMessage, const uint8_t *const start, const struct PacketRead state) {
	uprintf("%s [netVersion=%hhu protocolVersion=%hhu beatUpVersion=%hhu gameVersion=%s windowSize=%u]", errorMessage,
		state.context.netVersion, state.context.protocolVersion, state.context.beatUpVersion, reflect(GameVersion, state.context.gameVersion), state.context.windowSize);
	if(state.traceDepth == 0)
		uprintf(" (expected %zu, read %zu)", state.end - start, *state.head - start);
	if(state.traceDepth != 0) {
		uprintf("\n    at: %s", trace->stack[0]);
		for(unsigned i = 1; i < state.traceDepth && i < lengthof(trace->stack); ++i)
			uprintf(" > %s", trace->stack[i]);
		if(state.traceDepth > lengthof(trace->stack))
			uprintf(" > ...");
	}
	uprintf("\n    ");
	for(const uint8_t *it = start; it < state.end; ++it)
		uprintf("%02hhx", *it);
	if(*state.head < state.end) {
		uprintf("\n    ");
		for(const uint8_t *it = start; it < *state.head; ++it)
			uprintf("  ");
		if(state.traceDepth != 0)
			uprintf("^ field starts here");
		else
			uprintf("^ extra data starts here");
	}
	uprintf("\n");
}
#define check_range(_state, _size) check_range_(_Generic((_state), struct PacketRead: PacketRead_toRead, struct PacketWrite: PacketWrite_toRead)(_state), _size)
static void check_range_(const struct PacketRead state, const size_t size) {
	if(state.end - *state.head >= (ptrdiff_t)size)
		return;
	assert(trace != NULL);
	pkt_dump("Unexpected end of packet", trace->start, state);
	longjmp(trace->fail, 1);
}
#define check_overflow(_state, _fieldStart, _count, _limit, _field) \
	check_overflow_(_Generic((_state), struct PacketRead: PacketRead_toRead, struct PacketWrite: PacketWrite_toRead)(_state), _fieldStart, _count, _limit, _field)
static uint32_t check_overflow_(struct PacketRead state, const uint8_t *fieldStart, const uint32_t count, const uint32_t limit, const char field[const]) {
	if(count <= limit)
		return count;
	char message[256];
	message[0] = '\0';
	snprintf(message, lengthof(message), "Overflow in read of `%s`", field);
	assert(trace != NULL);
	state.head = &fieldStart;
	pkt_dump(message, trace->start, state);
	longjmp(trace->fail, 1);
}
static_assert(sizeof(uint8_t) == 1, "");
#define _pkt_b_read(data, state) _pkt_u8_read((uint8_t*)(data), state)
#define _pkt_i8_read(data, state) _pkt_u8_read((uint8_t*)(data), state)
#define _pkt_char_read(data, state) _pkt_u8_read((uint8_t*)(data), state)
static void _pkt_u8_read(uint8_t *const restrict data, const struct PacketRead state) {
	check_range(state, sizeof(*data));
	*data = *(*state.head)++;
}
static_assert(sizeof(uint16_t) == 2, "");
#define _pkt_i16_read(data, state) _pkt_u16_read((uint16_t*)(data), state)
[[maybe_unused]] static void _pkt_u16_read(uint16_t *const restrict data, const struct PacketRead state) {
	check_range(state, sizeof(*data));
	*data = (*state.head)[0] | (uint16_t)((*state.head)[1] << 8);
	*state.head += sizeof(*data);
}
static_assert(sizeof(uint32_t) == 4, "");
#define _pkt_i32_read(data, state) _pkt_u32_read((uint32_t*)(data), state)
[[maybe_unused]] static void _pkt_u32_read(uint32_t *const restrict data, const struct PacketRead state) {
	check_range(state, sizeof(*data));
	*data = (*state.head)[0] | (uint32_t)(*state.head)[1] << 8 | (uint32_t)(*state.head)[2] << 16 | (uint32_t)(*state.head)[3] << 24;
	*state.head += sizeof(*data);
}
static_assert(sizeof(uint64_t) == 8, "");
#define _pkt_i64_read(data, state) _pkt_u64_read((uint64_t*)(data), state)
[[maybe_unused]] static void _pkt_u64_read(uint64_t *const restrict data, const struct PacketRead state) {
	check_range(state, sizeof(*data));
	*data = (uint64_t)(*state.head)[0] | (uint64_t)(*state.head)[1] << 8 | (uint64_t)(*state.head)[2] << 16 | (uint64_t)(*state.head)[3] << 24 |
		(uint64_t)(*state.head)[4] << 32 | (uint64_t)(*state.head)[5] << 40 | (uint64_t)(*state.head)[6] << 48 | (uint64_t)(*state.head)[7] << 56;
	*state.head += sizeof(*data);
}
static void _pkt_vu64_read(uint64_t *restrict data, const struct PacketRead state) {
	*data = 0;
	uint8_t byte, shift = 0;
	const uint8_t *const start = *state.head;
	do {
		check_overflow(state, start, shift, 63, "vu64");
		_pkt_u8_read(&byte, state);
		*data |= (byte & 127llu) << shift;
		shift += 7;
	} while(byte & 128);
}
static void _pkt_vi64_read(int64_t *restrict data, const struct PacketRead state) {
	_pkt_vu64_read((uint64_t*)data, state);
	bool negative = ((*data & 1ll) == 1ll);
	*data >>= 1;
	if(negative)
		*data = 1ll - *data;
}
[[maybe_unused]] static void _pkt_vu32_read(uint32_t *restrict data, const struct PacketRead state) {
	uint64_t tmp;
	_pkt_vu64_read(&tmp, state);
	*data = (uint32_t)tmp;
}
[[maybe_unused]] static void _pkt_vi32_read(int32_t *restrict data, const struct PacketRead state) {
	int64_t tmp;
	_pkt_vi64_read(&tmp, state);
	*data = (int32_t)tmp;
}
[[maybe_unused]] static void _pkt_raw_read(uint8_t *restrict data, const size_t count, const struct PacketRead state) {
	check_range(state, count);
	memcpy(data, *state.head, count);
	*state.head += count;
}
static_assert(sizeof(float) == 4, "");
static inline void _pkt_f32_read(float *const data_out, const struct PacketRead state) {
	_pkt_raw_read((uint8_t*)data_out, sizeof(*data_out), state);
}
static_assert(sizeof(double) == 8, "");
static inline void _pkt_f64_read(double *const data_out, const struct PacketRead state) {
	_pkt_raw_read((uint8_t*)data_out, sizeof(*data_out), state);
}

#define _pkt_b_write(data, state) _pkt_u8_write((const uint8_t*)(data), state)
#define _pkt_i8_write(data, state) _pkt_u8_write((const uint8_t*)(data), state)
#define _pkt_char_write(data, state) _pkt_u8_write((const uint8_t*)(data), state)
static void _pkt_u8_write(const uint8_t *restrict data, const struct PacketWrite state) {
	check_range(state, sizeof(*data));
	*(*state.head)++ = *data;
}
#define _pkt_i16_write(data, state) _pkt_u16_write((const uint16_t*)(data), state)
[[maybe_unused]] static void _pkt_u16_write(const uint16_t *restrict data, const struct PacketWrite state) {
	check_range(state, sizeof(*data));
	*(*state.head)++ = *data & 255;
	*(*state.head)++ = *data >> 8 & 255;
}
#define _pkt_i32_write(data, state) _pkt_u32_write((const uint32_t*)(data), state)
[[maybe_unused]] static void _pkt_u32_write(const uint32_t *restrict data, const struct PacketWrite state) {
	check_range(state, sizeof(*data));
	*(*state.head)++ = *data & 255;
	*(*state.head)++ = *data >> 8 & 255;
	*(*state.head)++ = *data >> 16 & 255;
	*(*state.head)++ = *data >> 24 & 255;
}
#define _pkt_i64_write(data, state) _pkt_u64_write((const uint64_t*)(data), state)
[[maybe_unused]] static void _pkt_u64_write(const uint64_t *restrict data, const struct PacketWrite state) {
	check_range(state, sizeof(*data));
	*(*state.head)++ = *data & 255;
	*(*state.head)++ = *data >> 8 & 255;
	*(*state.head)++ = *data >> 16 & 255;
	*(*state.head)++ = *data >> 24 & 255;
	*(*state.head)++ = *data >> 32 & 255;
	*(*state.head)++ = *data >> 40 & 255;
	*(*state.head)++ = *data >> 48 & 255;
	*(*state.head)++ = *data >> 56 & 255;
}
static void _pkt_vu64_write(const uint64_t *restrict data, const struct PacketWrite state) {
	uint64_t v = *data;
	for(; v >= 128; v >>= 7)
		_pkt_u8_write((uint8_t[]){(uint8_t)v | 128u}, state);
	_pkt_u8_write((uint8_t[]){(uint8_t)v}, state);
}
static void _pkt_vi64_write(const int64_t *restrict data, const struct PacketWrite state) {
	uint64_t v = (*data < 0) ? (uint64_t)((-(*data + 1) << 1) + 1) : (uint64_t)*data << 1;
	_pkt_vu64_write(&v, state);
}
[[maybe_unused]] static void _pkt_vu32_write(const uint32_t *restrict data, const struct PacketWrite state) {
	_pkt_vu64_write((uint64_t[]){*data}, state);
}
[[maybe_unused]] static void _pkt_vi32_write(const int32_t *restrict data, const struct PacketWrite state) {
	_pkt_vi64_write((int64_t[]){*data}, state);
}
[[maybe_unused]] static inline void _pkt_raw_write(const uint8_t *restrict data, size_t count, const struct PacketWrite state) {
	check_range(state, count);
	memcpy(*state.head, data, count);
	*state.head += count;
}
static inline void _pkt_f32_write(const float *const data, const struct PacketWrite state) {
	_pkt_raw_write((const uint8_t*)data, sizeof(*data), state);
}
static inline void _pkt_f64_write(const double *const data, const struct PacketWrite state) {
	_pkt_raw_write((const uint8_t*)data, sizeof(*data), state);
}

[[maybe_unused]] static void _pkt_Cookie32_read(struct Cookie32 *restrict data, const struct PacketRead parent) {
	_pkt_raw_read(data->raw, lengthof(data->raw), scope(parent, "Cookie32"));
}
[[maybe_unused]] static void _pkt_Cookie32_write(const struct Cookie32 *restrict data, const struct PacketWrite parent) {
	_pkt_raw_write(data->raw, lengthof(data->raw), scope(parent, "Cookie32"));
}

static uint32_t _pkt_BaseString_read(const struct PacketRead parent) {
	const struct PacketRead state = scope(parent, "BaseString");
	if(state.context.netVersion < 12) {
		uint32_t length32;
		_pkt_u32_read(&length32, state);
		return length32 + 1;
	}
	uint16_t length16;
	_pkt_u16_read(&length16, state);
	return length16;
}

#define STRING_RDWR_FUNC(_type) \
	[[maybe_unused]] static void _pkt_##_type##_read(struct _type *restrict data, const struct PacketRead parent) { \
		const struct PacketRead state = scope(parent, #_type); \
		const uint8_t *const start = *state.head; \
		data->length = _pkt_BaseString_read(state); \
		data->isNull = (data->length == 0); \
		data->length -= !data->isNull; \
		check_overflow(state, start, data->length, sizeof(data->data), #_type ".data"); \
		_pkt_raw_read((uint8_t*)data->data, data->length, state); \
	} \
	[[maybe_unused]] static void _pkt_##_type##_write(const struct _type *restrict data, const struct PacketWrite parent) { \
		const struct PacketWrite state = scope(parent, #_type); \
		if(state.context.netVersion < 12) \
			_pkt_u32_write(&data->length, state); \
		else \
			_pkt_u16_write((uint16_t[]){(uint16_t)data->length + !data->isNull}, state); \
		_pkt_raw_write((const uint8_t*)data->data, data->length, state); \
	}
STRING_RDWR_FUNC(String)
STRING_RDWR_FUNC(LongString)
#undef STRING_RDWR_FUNC
