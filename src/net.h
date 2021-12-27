#include "packets.h"
#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

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

struct SS {
	socklen_t len;
	union {
		struct sockaddr_storage ss;
		struct sockaddr sa;
		struct sockaddr_in in;
		struct sockaddr_in6 in6;
	};
};

struct MasterServerSession;

struct NetContext {
	int32_t NET_H_PRIVATE(sockfd);
	mbedtls_ctr_drbg_context NET_H_PRIVATE(ctr_drbg);
	mbedtls_entropy_context NET_H_PRIVATE(entropy);
	mbedtls_ecp_group NET_H_PRIVATE(grp);
	struct MasterServerSession *NET_H_PRIVATE(sessionList);
	struct MasterServerSession *(*NET_H_PRIVATE(onConnect))(struct NetContext *ctx, struct SS addr);
	const uint8_t *NET_H_PRIVATE(dirt);
};

uint8_t *MasterServerSession_get_clientRandom(struct MasterServerSession *session);
const uint8_t *MasterServerSession_get_serverRandom(const struct MasterServerSession *session);
const uint8_t *MasterServerSession_get_cookie(const struct MasterServerSession *session);
_Bool MasterServerSession_write_key(const struct MasterServerSession *session, struct NetContext *ctx, uint8_t *out, uint32_t *out_len);
_Bool MasterServerSession_signature(const struct MasterServerSession *session, struct NetContext *ctx, const mbedtls_pk_context *key, const uint8_t *in, uint32_t in_len, struct ByteArrayNetSerializable *out);
_Bool MasterServerSession_set_clientPublicKey(struct MasterServerSession *session, struct NetContext *ctx, const struct ByteArrayNetSerializable *in);
void MasterServerSession_set_epoch(struct MasterServerSession *session, uint32_t epoch);
_Bool MasterServerSession_change_state(struct MasterServerSession *session, HandshakeMessageType old, HandshakeMessageType new);
void MasterServerSession_set_state(struct MasterServerSession *session, HandshakeMessageType state);
char *MasterServerSession_get_gameId(struct MasterServerSession *session);
uint32_t MasterServerSession_get_lastKeepAlive(struct MasterServerSession *session);
uint32_t *MasterServerSession_ClientHelloWithCookieRequest_requestId(struct MasterServerSession *session);
struct SS MasterServerSession_get_addr(struct MasterServerSession *session);

_Bool net_init(struct NetContext *ctx, uint16_t port, struct MasterServerSession *(*onConnect)(struct NetContext *ctx, struct SS addr));
void net_stop(struct NetContext *ctx);
void net_cleanup(struct NetContext *ctx);
struct MasterServerSession *net_resolve_session(struct NetContext *ctx, struct SS addr);
struct MasterServerSession *net_create_session(struct NetContext *ctx, struct SS addr);
_Bool net_reset_session(struct NetContext *ctx, struct MasterServerSession *session);
uint32_t net_recv(struct NetContext *ctx, uint8_t *buf, uint32_t buf_len, struct MasterServerSession **session, PacketProperty *property, uint8_t **pkt);
void net_send(struct NetContext *ctx, struct MasterServerSession *session, PacketProperty property, const uint8_t *buf, uint32_t len, _Bool reliable);
_Bool net_handle_ack(struct MasterServerSession *session, struct MessageHeader *message_out, struct SerializeHeader *serial_out, uint32_t requestId);
uint32_t net_getNextRequestId(struct MasterServerSession *session);
int32_t net_get_sockfd(struct NetContext *ctx);
mbedtls_ctr_drbg_context *net_get_ctr_drbg(struct NetContext *ctx);

uint32_t net_time();

_Bool status_init(const char *path, uint16_t port);
void status_cleanup();

_Bool status_ssl_init(mbedtls_x509_crt *cert, mbedtls_pk_context *key, const char *path, uint16_t port);
void status_ssl_cleanup();

_Bool master_init(mbedtls_x509_crt *cert, mbedtls_pk_context *key, uint16_t port);
void master_cleanup();

void instance_init(const char *domain);
void instance_cleanup();
