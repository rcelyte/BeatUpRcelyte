#pragma once
#include "../net.h"
#include "../packets.h"
#include "../wire.h"

void pool_reset(void);

struct PoolHost;
struct PoolHost *pool_host_attach(struct WireLink *link);
void pool_host_detach(struct PoolHost *host);
void TEMPpool_host_setAttribs(struct PoolHost *host, uint32_t capacity, bool discover);
struct WireLink *pool_host_wire(struct PoolHost *host);

struct PoolHost *pool_handle_new(uint32_t *room_out, bool random);
struct PoolHost *pool_handle_new_named(uint32_t *room_out, ServerCode code);
void pool_handle_free(struct PoolHost *host, uint32_t room);
ServerCode pool_handle_code(struct PoolHost *host, uint32_t room);
struct PoolHost *pool_handle_lookup(uint32_t *room_out, ServerCode code);
