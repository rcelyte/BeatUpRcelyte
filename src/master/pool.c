#include "pool.h"
#include "../counter.h"
#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_SERVER_CODE 62193780
#define POOL_BLOCK_COUNT ((MAX_SERVER_CODE+256*16) / (256*16))

struct PoolHost {
	union WireLink *link;
	bool discover;
	uint16_t capacity;
	struct Counter64 blocks;
	ServerCode *codes;
};

#define CLEAR_POOLHOST (struct PoolHost){NULL, false, 0, {0}, NULL}

static uint32_t hosts_len = 0, nextSlot = 0;
static struct PoolHost *hosts = NULL;

static bool pool_grow(uint32_t newLength) {
	struct PoolHost *newHosts = realloc(hosts, newLength * sizeof(*hosts));
	if(!newHosts) {
		uprintf("realloc error\n");
		return true;
	}
	hosts = newHosts;
	while(hosts_len < newLength)
		hosts[hosts_len++] = CLEAR_POOLHOST;
	return false;
}

void pool_reset(struct NetContext *self) {
	for(uint32_t i = 0; i < hosts_len; ++i)
		if(hosts[i].link)
			wire_disconnect(self, hosts[i].link);
	free(hosts);
	hosts_len = 0, nextSlot = 0, hosts = NULL;
}

struct PoolHost *pool_host_attach(union WireLink *link) {
	if(nextSlot >= hosts_len)
		if(pool_grow(hosts_len ? (hosts_len * 2) : 8))
			return NULL;
	struct PoolHost *host = &hosts[nextSlot];
	*host = (struct PoolHost){
		.link = link,
		.discover = false,
		.capacity = 0,
		.blocks = {0},
		.codes = NULL,
	};
	while(++nextSlot < hosts_len)
		if(hosts[nextSlot].link == NULL)
			break;
	uprintf("pool_host_attach(): nextSlot %zd -> %u\n", host - hosts, nextSlot);
	return host;
}

void pool_host_detach(struct PoolHost *host) {
	if(host >= &hosts[hosts_len] || host->link == NULL)
		return;
	free(host->codes);
	*host = CLEAR_POOLHOST;
	if((uint32_t)(host - hosts) < nextSlot)
		nextSlot = (uint32_t)(host - hosts);
}

void TEMPpool_host_setAttribs(struct PoolHost *host, uint32_t capacity, bool discover) { // TODO: `TEMP` since calling multiple times for the same host will break things
	capacity &= ~1u; // round down to power of 2 for alignment
	host->codes = malloc(capacity * sizeof(*hosts->codes));
	if(!host->codes) {
		*host = CLEAR_POOLHOST;
		uprintf("alloc error\n");
		return;
	}
	host->discover = discover;
	host->capacity = capacity;
	host->blocks.bits = ~0llu;
	for(ServerCode *it = host->codes, *end = &host->codes[capacity]; it < end; ++it)
		*it = ServerCode_NONE;
}

union WireLink *pool_host_wire(struct PoolHost *host) {
	return host->link;
}

struct PoolHost *pool_host_lookup(union WireLink *link) {
	for(struct PoolHost *host = hosts; host < &hosts[hosts_len]; ++host)
		if(host->link == link)
			return host;
	return NULL;
}

static uint32_t globalRoomCount = 0;

static struct PoolHost *_pool_handle_new(uint32_t *room_out, ServerCode code) {
	struct PoolHost *host = &hosts[0]; // TODO: multiple hosts + load balancing
	if(!host->discover || Counter64_isEmpty(host->blocks)) {
		uprintf("Error: instance not available\n");
		return NULL;
	}
	uint32_t block = __builtin_ctzll(host->blocks.bits), freeCount = 0;
	for(uint32_t start = host->capacity * block / 64, i = host->capacity * (block + 1) / 64; i > start;)
		if(host->codes[--i] == ServerCode_NONE)
			*room_out = i, ++freeCount;
	if(freeCount < 2)
		Counter64_clear(&host->blocks, block);
	host->codes[*room_out] = code;
	++globalRoomCount, uprintf("%u room%s open\n", globalRoomCount, (globalRoomCount == 1) ? "" : "s");
	return host;
}

struct PoolHost *pool_handle_new(uint32_t *room_out, bool random) { // TODO: fix performance here utilizing hash lookups
	if(random) {
		uprintf("TODO: randomized room codes\n");
		return NULL;
	}
	for(ServerCode code = 0; code < 16; ++code) {
		if(code == ServerCode_NONE) // `ServerCode_NONE` is an assumed opaque value
			continue;
		if(!pool_handle_lookup((uint32_t[]){0}, code))
			return _pool_handle_new(room_out, code);
	}
	return NULL;
}

struct PoolHost *pool_handle_new_named(uint32_t *room_out, ServerCode code) {
	if(code == ServerCode_NONE || pool_handle_lookup((uint32_t[]){0}, code))
		return NULL;
	return _pool_handle_new(room_out, code);
}

void pool_handle_free(struct PoolHost *host, uint16_t room) {
	Counter64_set(&host->blocks, room * 64 / host->capacity);
	if(pool_handle_code(host, room) == ServerCode_NONE)
		return;
	host->codes[room] = ServerCode_NONE;
	--globalRoomCount, uprintf("%u room%s open\n", globalRoomCount, (globalRoomCount == 1) ? "" : "s");
}

ServerCode pool_handle_code(struct PoolHost *host, uint32_t room) {
	return host->codes[room];
}

struct PoolHost *pool_handle_lookup(uint32_t *room_out, ServerCode code) { // TODO: use a hash table for this
	for(struct PoolHost *host = hosts; host < &hosts[hosts_len]; ++host) {
		for(ServerCode *room = host->codes, *codes_end = &host->codes[host->capacity]; room < codes_end; ++room) {
			if(*room != code)
				continue;
			*room_out = room - host->codes;
			return host;
		}
	}
	return NULL;
}
