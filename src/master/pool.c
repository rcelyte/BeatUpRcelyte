#include "pool.h"
#include "../counter.h"
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

struct PoolHost {
	struct PoolHost *next;
	struct WireLink *link;
	HostCode ident;
	bool discover;
	uint32_t capacity;
	struct Counter64 blocks;
	ServerCode *codes;
};

static uint16_t lastId = 0;
static struct PoolHost *firstHost = NULL;

void pool_reset() {
	while(firstHost != NULL) {
		WireLink_free(firstHost->link); // TODO: this doesn't belong here
		pool_host_detach(firstHost);
	}
}

static bool ReservedId(uint16_t ident) {
	for(struct PoolHost *it = firstHost; it != NULL; it = it->next)
		if(it->ident == ident)
			return true;
	return false;
}

struct PoolHost *pool_host_attach(struct WireLink *link) {
	struct PoolHost *const host = malloc(sizeof(struct PoolHost));
	if(host == NULL)
		return NULL;
	for(uint32_t i = 0; i < 65536; ++i)
		if(!ReservedId(++lastId))
			break;
	*host = (struct PoolHost){
		.next = firstHost,
		.link = link,
		.ident = lastId,
	};
	firstHost = host;
	uprintf("pool_host_attach()\n");
	return host;
}

void pool_host_detach(struct PoolHost *host) {
	for(struct PoolHost **it = &firstHost; *it != NULL; it = &(*it)->next) {
		if(*it != host)
			continue;
		struct PoolHost *const host = *it;
		*it = host->next;
		free(host->codes);
		free(host);
		return;
	}
}

void TEMPpool_host_setAttribs(struct PoolHost *host, uint32_t capacity, bool discover) { // TODO: `TEMP` since calling multiple times for the same host will break things
	free(host->codes);
	*host = (struct PoolHost){
		.next = host->next,
		.link = host->link,
		.discover = discover,
		.capacity = capacity,
		.blocks.bits = ~UINT64_C(0),
		.codes = malloc(capacity * sizeof(*host->codes)),
	};
	if(host->codes == NULL) {
		host->discover = false;
		host->capacity = 0;
		uprintf("alloc error\n");
		return;
	}
	for(ServerCode *it = host->codes, *end = &host->codes[capacity]; it < end; ++it)
		*it = ServerCode_NONE;
}

struct WireLink *pool_host_wire(struct PoolHost *host) {
	return host->link;
}

HostCode pool_host_ident(struct PoolHost *host) {
	return host->ident;
}

struct PoolHost *pool_host_iter_start(void) {
	return firstHost;
}

struct PoolHost *pool_host_iter_next(struct PoolHost *current) {
	return current->next;
}

static uint32_t globalRoomCount = 0;
static struct PoolHost *_pool_handle_new(uint32_t *room_out, ServerCode code) {
	struct PoolHost *host = firstHost; // TODO: multiple hosts + load balancing
	uint32_t block = Counter64_get_next(host->blocks), freeCount = 0;
	if(!host->discover || block == COUNTER64_INVALID) {
		uprintf("Error: instance not available\n");
		return NULL;
	}
	for(uint32_t start = host->capacity * block / 64, i = host->capacity * (block + 1) / 64; i > start;) {
		if(host->codes[--i] != ServerCode_NONE) 
			continue;
		*room_out = i;
		++freeCount;
	}
	if(freeCount < 2)
		Counter64_clear(&host->blocks, block);
	host->codes[*room_out] = code;
	++globalRoomCount;
	uprintf("%u room%s open\n", globalRoomCount, (globalRoomCount == 1) ? "" : "s");
	return host;
}

// TODO: hash table lookups
struct PoolHost *pool_handle_new(uint32_t *room_out, bool random) {
	if(random) {
		uprintf("TODO: randomized room codes\n");
		return NULL;
	}
	assert(ServerCode_NONE == 0);
	for(ServerCode code = 1; code < 257; ++code)
		if(!pool_handle_lookup((uint32_t[]){0}, code))
			return _pool_handle_new(room_out, code);
	return NULL;
}

struct PoolHost *pool_handle_new_named(uint32_t *room_out, ServerCode code) {
	if(code == ServerCode_NONE || pool_handle_lookup((uint32_t[]){0}, code))
		return NULL;
	return _pool_handle_new(room_out, code);
}

void pool_handle_free(struct PoolHost *host, uint32_t room) {
	Counter64_set(&host->blocks, room * 64 / host->capacity);
	if(pool_handle_code(host, room) == ServerCode_NONE)
		return;
	host->codes[room] = ServerCode_NONE;
	--globalRoomCount;
	uprintf("%u room%s open\n", globalRoomCount, (globalRoomCount == 1) ? "" : "s");
}

ServerCode pool_handle_code(struct PoolHost *host, uint32_t room) {
	return host->codes[room];
}

struct PoolHost *pool_handle_lookup(uint32_t *room_out, ServerCode code) { // TODO: use a hash table for this
	for(struct PoolHost *host = firstHost; host != NULL; host = host->next) {
		for(ServerCode *room = host->codes, *codes_end = &host->codes[host->capacity]; room < codes_end; ++room) {
			if(*room != code)
				continue;
			*room_out = (uint32_t)(room - host->codes);
			return host;
		}
	}
	return NULL;
}
