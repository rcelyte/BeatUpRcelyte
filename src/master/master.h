#pragma once
#include "../net.h"
#include "../wire.h"
#include "../packets.h"
#include "../instance/eenet.h"
#include <mbedtls/x509_crt.h>
#include <mbedtls/ssl_cookie.h>

static constexpr uint32_t CONNECT_TIMEOUT_MS = 30000, IDLE_TIMEOUT_MS = 10000;

struct MasterSession;
struct MasterContext {
	mbedtls_ssl_cookie_ctx cookie;
	mbedtls_ssl_config config;
	mbedtls_x509_crt cert; // shallow copy
	mbedtls_pk_context key; // shallow copy
	struct MasterSession *sessionList;
};

struct GraphAuthToken {
	struct ConnectMessage base;
	ENetHost *enet;
	struct EENetPacket event;
};

struct WireContext *master_init(const mbedtls_x509_crt *cert, const mbedtls_pk_context *key, uint16_t port);
void master_cleanup(void);

void MasterContext_init(struct MasterContext *ctx, mbedtls_ctr_drbg_context *ctr_drbg);
bool MasterContext_setCertificate(struct MasterContext *ctx, const mbedtls_x509_crt *cert, const mbedtls_pk_context *key);
void MasterContext_cleanup(struct MasterContext *ctx);
bool MasterContext_handle(struct MasterContext *ctx, struct NetContext *net, struct MasterSession *session, const uint8_t *data, const uint8_t *end, struct GraphAuthToken *auth_out);
bool MasterContext_handleMessage(struct MasterContext *ctx, struct NetContext *net, struct MasterSession *session, struct UnconnectedMessage header, const uint8_t *data, const uint8_t *end, struct GraphAuthToken *auth_out);
struct MasterSession *MasterContext_lookup(struct MasterContext *ctx, struct SS addr);
struct NetSession *MasterContext_onResolve(struct MasterContext *ctx, struct NetContext *net, struct SS addr, const uint8_t packet[static 1536], uint32_t packet_len, uint8_t out[static 1536], uint32_t *out_len);
uint32_t MasterContext_onResend(struct MasterContext *ctx, struct NetContext *net, uint32_t currentTime);
