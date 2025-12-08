#include "pool.h"
#include "../counter.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>

struct PoolHost_Room {
	uint32_t sequence:30;
	bool managed:1, open:1;
};

struct PoolHost {
	struct PoolHost *next;
	struct WireLink *link;
	HostCode ident;
	bool discover;
	bool allManagedRoomsFull; // TODO: handle notify from instance
	uint32_t capacity;
	struct Counter64 blocks;
	ServerCode *codes;
	struct PoolHost_Room rooms[];
};

static uint32_t globalSequence = 0;
static uint16_t lastId = 0;
static struct PoolHost *firstHost = NULL;

static uint8_t RoomCodeFilter[4329488];
[[gnu::constructor]] static void pool_init_filter_() {
	static const uint8_t code_filter_bin[] = {
		#embed "code_filter.bin"
	};
	uint8_t *out = RoomCodeFilter;
	for(const uint8_t *head = code_filter_bin; head < endof(code_filter_bin);) {
		uint32_t span = 0, shift = 0;
		do {
			span |= (*head & 127llu) << shift;
			shift += 7;
		} while(*head++ & 128);
		const uint32_t byte = (span & 1) * 0xff, length = span >> 1;
		memset(out, (int32_t)byte, length);
		out += length;
		*out++ = *head++;
	}
	assert(out == endof(RoomCodeFilter));
}

static bool ServerCode_isMasked(const ServerCode code) {
	return code / 64 < lengthof(RoomCodeFilter) && (RoomCodeFilter[code / 64] & UINT64_C(1) << (code % 64)) != 0;
}

void pool_reset() {
	while(firstHost != NULL) {
		WireLink_free(firstHost->link); // TODO: this doesn't belong here
		pool_host_detach(firstHost);
	}
}

static bool ReservedId(uint16_t ident) {
	for(struct PoolHost *host = pool_host_iter_start(); host != NULL; host = pool_host_iter_next(host))
		if(host->ident == ident)
			return true;
	return false;
}

struct PoolHost *pool_host_attach(struct WireLink *const link, const uint32_t capacity, const bool discover) {
	static_assert(ServerCode_NONE == 0);
	struct PoolHost *const host = calloc(sizeof(struct PoolHost) + capacity * sizeof(*host->rooms) + capacity * sizeof(*host->codes), 1);
	if(host == NULL)
		return NULL;
	for(uint32_t i = 0; i < UINT16_MAX; ++i)
		if(!ReservedId(++lastId))
			break;
	*host = (struct PoolHost){
		.next = firstHost,
		.link = link,
		.ident = lastId,
		.discover = discover,
		.capacity = capacity,
		.blocks.bits = UINT64_MAX,
		.codes = (ServerCode*)&host->rooms[capacity],
	};
	firstHost = host;
	uprintf("pool_host_attach()\n");
	return host;
}

void pool_host_seal(struct PoolHost *const host) {
	host->discover = false;
}

void pool_host_detach(struct PoolHost *const host) {
	struct PoolHost **it = &firstHost;
	while(*it != host) {
		if(*it == NULL)
			return;
		it = &(*it)->next;
	}
	*it = host->next;
	free(host);
}

struct WireLink *pool_host_wire(struct PoolHost *const host) {
	return host->link;
}

HostCode pool_host_ident(struct PoolHost *const host) {
	return host->ident;
}

bool pool_host_isFull(struct PoolHost *const host) {
	return !host->discover || Counter64_get_next(host->blocks) == COUNTER64_INVALID;
}

struct PoolHost *pool_host_iter_start(void) {
	return firstHost;
}

struct PoolHost *pool_host_iter_next(struct PoolHost *current) {
	return current->next;
}

static uint32_t globalRoomCount = 0;
static uint32_t pool_handle_open(struct PoolHost *const host, const uint32_t managedRoom, ServerCode code) {
	const uint8_t block = (managedRoom != UINT32_MAX) ? (managedRoom * 64 / host->capacity) : Counter64_get_next(host->blocks);
	uint32_t room = managedRoom, freeCount = 0;
	for(uint32_t start = host->capacity * block / 64, i = host->capacity * (block + 1) / 64; i > start;) {
		if(host->rooms[--i].open)
			continue;
		if(room == UINT32_MAX)
			room = i;
		++freeCount;
	}
	assert(freeCount != 0);
	if(freeCount == 1)
		Counter64_clear(&host->blocks, block);
	assert(!host->rooms[room].open);
	host->codes[room] = code;
	host->rooms[room] = (struct PoolHost_Room){
		.sequence = ++globalSequence,
		.managed = (managedRoom != UINT32_MAX),
		.open = true,
	};
	++globalRoomCount;
	uprintf("%u room%s open\n", globalRoomCount, (globalRoomCount == 1) ? "" : "s");
	return room;
}

// TODO: hash table lookups
struct PoolHost *pool_handle_new(uint32_t *const room_out, mbedtls_ctr_drbg_context *const ctr_drbg, ServerCode code) {
	static_assert(ServerCode_NONE == 0);
	const bool named = (code != ServerCode_NONE);
	if(!named && ctr_drbg != NULL) { // private
		const ServerCode start = ServerCode_FromString("00000", 5), end = ServerCode_FromString("ZZZZZ", 5);
		unsigned attempt = 0;
		do {
			if(++attempt >= 193) {
				uprintf("Failed to find available room code\n");
				return NULL;
			}
			uint64_t random = 0;
			mbedtls_ctr_drbg_random(ctr_drbg, (uint8_t*)&random, sizeof(random));
			code = (ServerCode)((random % (end + 1 - start)) + start);
		} while(ServerCode_isMasked(code) || pool_handle_lookup((uint32_t[]){0}, code));
	} else { // public / named
		const ServerCode end = named ? code : ServerCode_FromString("ZZ", 2);
		code -= named;
		do {
			if(++code > end) {
				uprintf("%s\n", named ? "Requested room code not available" : "All public room codes in use");
				return NULL;
			}
		} while(ServerCode_isMasked(code) || pool_handle_lookup((uint32_t[]){0}, code));
	}
	struct PoolHost *host = pool_host_iter_start();
	do { // TODO: balance load instead of selecting first available instance
		if(host == NULL) {
			uprintf("Instance not available\n");
			return NULL;
		}
	} while(pool_host_isFull(host) && (host = pool_host_iter_next(host), true));
	*room_out = pool_handle_open(host, UINT32_MAX, code);
	return host;
}

ServerCode pool_handle_nameManaged(struct PoolHost *const host, const uint32_t room) {
	if(room >= host->capacity || host->rooms[room].managed)
		return pool_handle_code(host, room);
	ServerCode code = ServerCode_NONE;
	for(unsigned id = 0; id < 100; ++id) {
		if(id >= 100) {
			code = ServerCode_NONE;
			break;
		}
		code = ServerCode_FromString((const char[4]){'Q', 'P', '0' + (id / 10), '0' + (id % 10)}, 4);
	} while(pool_handle_lookup((uint32_t[]){0}, code));
	if(host->rooms[room].open) // instance has authority over its room list
		pool_handle_free(host, room);
	pool_handle_open(host, room, code);
	return code;
}

void pool_handle_free(struct PoolHost *const host, uint32_t room) {
	if(room >= host->capacity)
		return;
	Counter64_set(&host->blocks, room * 64 / host->capacity);
	if(!host->rooms[room].open)
		return;
	host->codes[room] = ServerCode_NONE;
	host->rooms[room] = (struct PoolHost_Room){};
	--globalRoomCount;
	uprintf("%u room%s open\n", globalRoomCount, (globalRoomCount == 1) ? "" : "s");
}

ServerCode pool_handle_code(const struct PoolHost *const host, const uint32_t room) {
	return (room < host->capacity) ? host->codes[room] : ServerCode_NONE;
}

uint32_t pool_handle_sequence(const struct PoolHost *const host, const uint32_t room) {
	return (room < host->capacity) ? host->rooms[room].sequence : 0;
}

bool pool_handle_isManaged(const struct PoolHost *const host, const uint32_t room) {
	return (room < host->capacity) ? host->rooms[room].managed : true;
}

struct PoolHost *pool_handle_lookup(uint32_t *const room_out, const ServerCode code) { // TODO: use a hash table for this
	struct PoolHost *host = pool_host_iter_start();
	if(code == ServerCode_NONE) {
		for(; host != NULL; host = pool_host_iter_next(host)) {
			// TODO: balance load instead of selecting first available instance; prioritize instances with `allManagedRoomsFull == false`
			if(Counter64_get_next(host->blocks) != COUNTER64_INVALID || !host->allManagedRoomsFull) {
				*room_out = UINT32_MAX;
				break;
			}
		}
	} else for(; host != NULL; host = pool_host_iter_next(host)) {
		static_assert(sizeof(*host->codes) == sizeof(wchar_t));
		const ServerCode *const room = (const ServerCode*)wmemchr((const wchar_t*)host->codes, code, host->capacity);
		if(room != NULL) {
			*room_out = (uint32_t)(room - host->codes);
			break;
		}
	}
	return host;
}
