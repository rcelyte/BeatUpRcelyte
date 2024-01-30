#include "pool.h"
#include "../counter.h"
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include "code_filter.h"

struct PoolHost {
	struct PoolHost *next;
	struct WireLink *link;
	HostCode ident;
	bool discover;
	uint32_t capacity;
	struct Counter64 blocks;
	ServerCode *codes;
	uint32_t *sequence;
};

static uint32_t globalSequence = 0;
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
		.sequence = malloc(capacity * sizeof(*host->sequence)),
	};
	if(host->codes == NULL || host->sequence == NULL) {
		free(host->codes);
		free(host->sequence);
		*host = (struct PoolHost){0};
		uprintf("alloc error\n");
		return;
	}
	for(ServerCode *it = host->codes, *end = &host->codes[capacity]; it < end; ++it)
		*it = ServerCode_NONE;
	memset(host->sequence, 0, capacity * sizeof(*host->sequence));
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
	host->sequence[*room_out] = ++globalSequence;
	++globalRoomCount;
	uprintf("%u room%s open\n", globalRoomCount, (globalRoomCount == 1) ? "" : "s");
	return host;
}

// TODO: hash table lookups
struct PoolHost *pool_handle_new(uint32_t *const room_out, mbedtls_ctr_drbg_context *const ctr_drbg) {
	assert(ServerCode_NONE == 0);
	if(ctr_drbg == NULL) {
		for(ServerCode code = 1, end = StringToServerCode("ZZ", 2); code <= end; ++code)
			if(!pool_handle_lookup((uint32_t[]){0}, code))
				return _pool_handle_new(room_out, code);
		uprintf("All public room codes in use\n");
		return NULL;
	}
	for(uint32_t attempt = 0; attempt < 192; ++attempt) {
		uint64_t code = 0;
		mbedtls_ctr_drbg_random(ctr_drbg, (uint8_t*)&code, sizeof(code));
		code = (code % StringToServerCode("ZZZZZ", 5)) + 1;
		if(code / 64 < lengthof(RoomCodeFilter) && (RoomCodeFilter[code / 64] & UINT64_C(1) << (code % 64)) != 0)
			continue;
		if(!pool_handle_lookup((uint32_t[]){0}, (ServerCode)code))
			return _pool_handle_new(room_out, (ServerCode)code);
	}
	uprintf("Failed to find an available room code\n");
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

ServerCode pool_handle_code(const struct PoolHost *const host, const uint32_t room) {
	return host->codes[room];
}

uint32_t pool_handle_sequence(const struct PoolHost *const host, const uint32_t room) {
	return host->sequence[room];
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
