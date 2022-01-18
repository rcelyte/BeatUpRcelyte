#include "pool.h"
#include "wire.h"
#include "scramble.h"
#include "packets.h"
#include <stddef.h>

#define MAX_SERVER_CODE 62193780
#define POOL_BLOCK_COUNT ((MAX_SERVER_CODE+256*16) / (256*16))

#define lengthof(x) (sizeof(x)/sizeof(*x))

struct RoomBlockPtr {
	struct WireBlockHandle handle;
	uint16_t idle;
	uint8_t high[16];
};
static struct RoomBlockPtr rooms[POOL_BLOCK_COUNT];
static uint16_t count = 0, alloc[POOL_BLOCK_COUNT];

static struct RoomHandle RoomHandle_code(ServerCode code) {
	struct RoomHandle h;
	h.sub = code & 15;
	code >>= 4;
	h.high = code / POOL_BLOCK_COUNT;
	h.block = code - h.high * POOL_BLOCK_COUNT;
	return h;
}

static _Bool pool_reserve_room(ServerCode code) { // DO NOT RESERVE MORE THAN 15 ROOMS
	struct RoomHandle h;
	pool_find_room(code, &h);
	if(rooms[h.block].idle == 0xffff)
		if(wire_request_block(&rooms[h.block].handle))
			return 1;
	rooms[h.block].idle &= ~(1 << h.sub);
	return 0;
}

_Bool pool_init() {
	for(uint32_t i = 0; i < lengthof(rooms); ++i)
		rooms[i] = (struct RoomBlockPtr){{0,0,0},0xffff,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
	for(uint32_t i = 0; i < lengthof(alloc); ++i)
		alloc[i] = i;
	reshuffle:
	scramble_init();
	ServerCode codes[] = {
		StringToServerCode(NULL, 0),
		StringToServerCode("INDEX", 5),
	};
	for(uint32_t a = 1; a < lengthof(codes); ++a) {
		for(uint32_t b = a; b < lengthof(codes); ++b) {
			struct RoomHandle ha, hb;
			pool_find_room(codes[a-1], &ha);
			pool_find_room(codes[b], &hb);
			if(ha.block == hb.block && ha.sub == hb.sub)
				goto reshuffle;
		}
	}
	for(uint32_t i = 0; i < lengthof(codes); ++i)
		if(pool_reserve_room(codes[i]))
			return 1;
	return 0;
}

_Bool pool_request_room(struct RoomHandle *out) {
	uint16_t block = alloc[count];
	if(rooms[block].idle == 0xffff)
		if(wire_request_block(&rooms[block].handle))
			return 1;
	out->block = block;
	out->sub = __builtin_ctz(rooms[out->block].idle);
	out->high = rooms[out->block].high[out->sub];
	rooms[out->block].idle &= rooms[out->block].idle - 1;
	count += (rooms[out->block].idle == 0);
	return 0;
}

void pool_room_close(struct RoomHandle h) {
	++rooms[h.block].high[h.sub];
	if(pool_room_code(h) > MAX_SERVER_CODE)
		rooms[h.block].high[h.sub] = 0;
	if(rooms[h.block].idle == 0)
		alloc[--count] = h.block;
	rooms[h.block].idle |= 1 << h.sub;
	if(rooms[h.block].idle == 0xffff)
		wire_block_release(rooms[h.block].handle);
}

ServerCode pool_room_code(struct RoomHandle h) {
	return ((h.high * POOL_BLOCK_COUNT + h.block) << 4) | h.sub;
}

_Bool pool_find_room(ServerCode code, struct RoomHandle *out) {
	out->sub = code & 15;
	code >>= 4;
	out->high = code / POOL_BLOCK_COUNT;
	out->block = code - out->high * POOL_BLOCK_COUNT;
	return out->high != rooms[out->block].high[out->sub];
}
