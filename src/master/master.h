#pragma once
#include "../net.h"
#include "../wire.h"
#include "../packets.h"
#include <mbedtls/x509_crt.h>

struct MasterSession;
struct MasterContext {
	const mbedtls_x509_crt *cert;
	const mbedtls_pk_context *key;
	struct MasterSession *sessionList;
	void (*onGraphAuth)(struct NetContext *net, struct NetSession *session, const struct AuthenticateGameLiftUserRequest *req);
	void *userptr;
};

struct WireContext *master_init(const mbedtls_x509_crt *cert, const mbedtls_pk_context *key, uint16_t port);
void master_cleanup(void);

void MasterContext_init(struct MasterContext *ctx);
bool MasterContext_setCertificate(struct MasterContext *ctx, const mbedtls_x509_crt *cert, const mbedtls_pk_context *key);
void MasterContext_cleanup(struct MasterContext *ctx);
void MasterContext_handle(struct MasterContext *ctx, struct NetContext *net, struct MasterSession *session, const uint8_t *data, const uint8_t *end);
struct NetSession *MasterContext_onResolve(struct MasterContext *ctx, struct NetContext *net, struct SS addr, const uint8_t packet[static 1536], uint32_t packet_len, uint8_t out[static 1536], uint32_t *out_len);
uint32_t MasterContext_onResend(struct MasterContext *ctx, struct NetContext *net, uint32_t currentTime);
