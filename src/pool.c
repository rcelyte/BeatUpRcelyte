#include "global.h"
#include "pool.h"
#include "scramble.h"
#include <stddef.h>
#include <pthread.h>

#define MAX_SERVER_CODE 62193780
#define POOL_BLOCK_COUNT ((MAX_SERVER_CODE+256*16) / (256*16))

#define lengthof(x) (sizeof(x)/sizeof(*(x)))

struct RoomBlockPtr {
	struct WireBlockHandle handle;
	uint16_t idle;
	uint8_t high[16];
};
static struct RoomBlockPtr rooms[POOL_BLOCK_COUNT];
static uint16_t count = 0, alloc[POOL_BLOCK_COUNT];
static pthread_mutex_t pool_mutex;

/*static struct RoomHandle RoomHandle_code(ServerCode code) {
	struct RoomHandle h;
	h.sub = code & 15;
	code >>= 4;
	h.high = code / POOL_BLOCK_COUNT;
	h.block = code - h.high * POOL_BLOCK_COUNT;
	return h;
}*/

static bool pool_reserve_room(ServerCode code) { // DO NOT RESERVE MORE THAN 15 ROOMS
	struct RoomHandle room;
	struct WireRoomHandle handle;
	pool_find_room(code, &room, &handle);
	if(rooms[room.block].idle == 0xffff)
		if(wire_request_block(&rooms[room.block].handle, room.block))
			return 1;
	rooms[room.block].idle &= ~(1 << room.sub);
	return 0;
}

bool pool_init() {
	if(pthread_mutex_init(&pool_mutex, NULL)) {
		uprintf("pthread_mutex_init() failed\n");
		return 1;
	}
	for(uint32_t i = 0; i < lengthof(rooms); ++i)
		rooms[i] = (struct RoomBlockPtr){{0,0,0},0xffff,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
	for(uint32_t i = 0; i < lengthof(alloc); ++i)
		alloc[i] = i;
	reshuffle:
	scramble_init();
	ServerCode codes[] = {
		StringToServerCode(NULL, 0),
		// StringToServerCode("INDEX", 5),
	};
	for(uint32_t a = 1; a < lengthof(codes); ++a) {
		for(uint32_t b = a; b < lengthof(codes); ++b) {
			struct RoomHandle roomA, roomB;
			struct WireRoomHandle handle;
			pool_find_room(codes[a-1], &roomA, &handle);
			pool_find_room(codes[b], &roomB, &handle);
			if(roomA.block == roomB.block && roomA.sub == roomB.sub)
				goto reshuffle;
		}
	}
	for(uint32_t i = 0; i < lengthof(codes); ++i)
		if(pool_reserve_room(codes[i]))
			return 1;
	return 0;
}

void pool_free() {
	if(pthread_mutex_destroy(&pool_mutex))
		uprintf("pthread_mutex_destroy() failed\n");
}

static uint32_t roomCount = 0;

bool pool_request_room(struct RoomHandle *room_out, struct WireRoomHandle *handle_out, struct String managerId, struct GameplayServerConfiguration configuration) {
	pthread_mutex_lock(&pool_mutex);
	uint16_t block = alloc[count];
	if(rooms[block].idle == 0xffff)
		if(wire_request_block(&rooms[block].handle, block))
			return 1;
	room_out->block = block;
	handle_out->block = rooms[block].handle;
	handle_out->sub = room_out->sub = __builtin_ctz(rooms[room_out->block].idle);
	room_out->high = rooms[room_out->block].high[room_out->sub];
	rooms[room_out->block].idle &= rooms[room_out->block].idle - 1;
	count += (rooms[room_out->block].idle == 0);
	uprintf("%u rooms open\n", ++roomCount);
	pthread_mutex_unlock(&pool_mutex);
	return wire_room_open(*handle_out, managerId, configuration, pool_room_code(*room_out));
}

void pool_room_close_notify(struct RoomHandle room) {
	pthread_mutex_lock(&pool_mutex);
	#ifdef SCRAMBLE_CODES
	++rooms[room.block].high[room.sub];
	if(pool_room_code(room) > MAX_SERVER_CODE)
		rooms[room.block].high[room.sub] = 0;
	#endif
	if(rooms[room.block].idle == 0)
		alloc[--count] = room.block;
	rooms[room.block].idle |= 1 << room.sub;
	if(rooms[room.block].idle == 0xffff)
		wire_block_release(rooms[room.block].handle);
	uprintf("%u rooms open\n", --roomCount);
	pthread_mutex_unlock(&pool_mutex);
}

ServerCode pool_room_code(struct RoomHandle room) {
	return ((room.high * POOL_BLOCK_COUNT + room.block) << 4) | room.sub;
}

bool pool_find_room(ServerCode code, struct RoomHandle *room_out, struct WireRoomHandle *handle_out) {
	handle_out->sub = room_out->sub = code & 15;
	code >>= 4;
	room_out->high = code / POOL_BLOCK_COUNT;
	room_out->block = code - room_out->high * POOL_BLOCK_COUNT;
	pthread_mutex_lock(&pool_mutex);
	handle_out->block = rooms[room_out->block].handle;
	bool notFound = ((rooms[room_out->block].idle >> room_out->sub) & 1) || room_out->high != rooms[room_out->block].high[room_out->sub];
	pthread_mutex_unlock(&pool_mutex);
	return notFound;
}
