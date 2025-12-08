#pragma once
#include "../net.h"
#include "../packets.h"
#include "../wire.h"

typedef uint16_t HostCode;

void pool_reset(void);

struct PoolHost;
struct PoolHost *pool_host_attach(struct WireLink *link, uint32_t capacity, bool discover);
void pool_host_seal(struct PoolHost *host);
void pool_host_detach(struct PoolHost *host);
struct WireLink *pool_host_wire(struct PoolHost *host);
HostCode pool_host_ident(struct PoolHost *host);
bool pool_host_isFull(struct PoolHost *host);

struct PoolHost *pool_host_iter_start(void);
struct PoolHost *pool_host_iter_next(struct PoolHost *current);

struct PoolHost *pool_handle_new(uint32_t *room_out, mbedtls_ctr_drbg_context *ctr_drbg, ServerCode code);
ServerCode pool_handle_nameManaged(struct PoolHost *host, uint32_t room);
void pool_handle_free(struct PoolHost *host, uint32_t room);
ServerCode pool_handle_code(const struct PoolHost *host, uint32_t room);
uint32_t pool_handle_sequence(const struct PoolHost *host, uint32_t room);
bool pool_handle_isManaged(const struct PoolHost *host, uint32_t room);
struct PoolHost *pool_handle_lookup(uint32_t *room_out, ServerCode code);
