#include "packets.h"
#include "encryption.h"
#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <stdatomic.h>

#ifdef WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#else
#include <arpa/inet.h>
#endif

#ifndef NET_H_PRIVATE
#define NET_H_PRIVATE(x) _p_##x##__LINE__
#endif

#define NET_MAX_SEQUENCE 32768
#define NET_WINDOW_SIZE 64
#define NET_RESEND_DELAY 27

struct SS {
	socklen_t len;
	union {
		struct sockaddr_storage ss;
		struct sockaddr sa;
		struct sockaddr_in in;
		struct sockaddr_in6 in6;
	};
};

uint8_t addrs_are_equal(const struct SS *a0, const struct SS *a1);
void net_tostr(const struct SS *a, char *out);

struct NetSession {
	uint8_t clientRandom[32];
	uint8_t NET_H_PRIVATE(serverRandom)[32];
	uint8_t NET_H_PRIVATE(cookie)[32];
	mbedtls_mpi NET_H_PRIVATE(serverSecret);
	mbedtls_ecp_point NET_H_PRIVATE(serverPublic);
	struct EncryptionState NET_H_PRIVATE(encryptionState);
	struct SS NET_H_PRIVATE(addr);
	uint32_t NET_H_PRIVATE(lastKeepAlive);
};

struct Context;
struct NetContext {
	int32_t NET_H_PRIVATE(sockfd);
	atomic_bool NET_H_PRIVATE(run);
	mbedtls_ctr_drbg_context NET_H_PRIVATE(ctr_drbg);
	mbedtls_entropy_context NET_H_PRIVATE(entropy);
	mbedtls_ecp_group NET_H_PRIVATE(grp);
	struct NetSession *NET_H_PRIVATE(sessionList);
	struct Context *user;
	struct NetSession *(*onResolve)(struct Context *ctx, struct SS addr);
	void (*onResend)(struct Context *ctx, uint32_t currentTime, uint32_t *nextTick);
	const uint8_t *NET_H_PRIVATE(dirt);
};

const uint8_t *NetSession_get_serverRandom(const struct NetSession *session);
const uint8_t *NetSession_get_cookie(const struct NetSession *session);
_Bool NetSession_write_key(const struct NetSession *session, struct NetContext *ctx, uint8_t *out, uint32_t *out_len);
_Bool NetSession_signature(const struct NetSession *session, struct NetContext *ctx, const mbedtls_pk_context *key, const uint8_t *in, uint32_t in_len, struct ByteArrayNetSerializable *out);
_Bool NetSession_set_clientPublicKey(struct NetSession *session, struct NetContext *ctx, const struct ByteArrayNetSerializable *in);
uint32_t NetSession_get_lastKeepAlive(struct NetSession *session);
const struct SS *NetSession_get_addr(struct NetSession *session);

_Bool net_init(struct NetContext *ctx, uint16_t port);
void net_stop(struct NetContext *ctx);
void net_cleanup(struct NetContext *ctx);
/*struct NetSession *net_resolve_session(struct NetContext *ctx, struct SS addr);
struct NetSession *net_create_session(struct NetContext *ctx, struct SS addr);*/
_Bool net_session_init(struct NetContext *ctx, struct NetSession *session, struct SS addr);
_Bool net_session_reset(struct NetContext *ctx, struct NetSession *session);
void net_session_free(struct NetSession *session);
uint32_t net_recv(struct NetContext *ctx, uint8_t *buf, uint32_t buf_len, struct NetSession **session, struct NetPacketHeader *header, const uint8_t **pkt);
void net_send(struct NetContext *ctx, struct NetSession *session, PacketProperty property, const uint8_t *buf, uint32_t len);
void net_send_mtu(struct NetContext *ctx, struct NetSession *session, PacketProperty property, const uint8_t *buf, uint32_t len);
void net_send_internal(struct NetContext *ctx, struct NetSession *session, PacketProperty property, const uint8_t *buf, uint32_t len);
int32_t net_get_sockfd(struct NetContext *ctx);
mbedtls_ctr_drbg_context *net_get_ctr_drbg(struct NetContext *ctx);

uint32_t net_time();

_Bool status_init(const char *path, uint16_t port);
void status_cleanup();

_Bool status_ssl_init(mbedtls_x509_crt *cert, mbedtls_pk_context *key, const char *path, uint16_t port);
void status_ssl_cleanup();

_Bool master_init(mbedtls_x509_crt *cert, mbedtls_pk_context *key, uint16_t port);
void master_cleanup();

_Bool instance_init(const char *domain);
void instance_cleanup();

static inline int32_t RelativeSequenceNumber(int32_t number, int32_t expected) {
	return (number - expected + NET_MAX_SEQUENCE + NET_MAX_SEQUENCE / 2) % NET_MAX_SEQUENCE - NET_MAX_SEQUENCE / 2;
}
