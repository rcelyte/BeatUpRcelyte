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

struct SS {
	socklen_t len;
	union {
		struct sockaddr_storage ss;
		struct sockaddr sa;
		struct sockaddr_in in;
		struct sockaddr_in6 in6;
	};
};

struct NetContext {
	int32_t sockfd;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
	struct SessionList *sessionList;
	ssize_t prev_size;
	uint8_t buf[262144];
};

struct MasterServerSession;

uint8_t *MasterServerSession_get_clientRandom(struct MasterServerSession *session);
uint8_t *MasterServerSession_get_serverRandom(struct MasterServerSession *session);
uint8_t *MasterServerSession_get_cookie(struct MasterServerSession *session);
_Bool MasterServerSession_write_key(struct MasterServerSession *session, uint8_t *out, uint32_t *out_len);
_Bool MasterServerSession_signature(struct MasterServerSession *session, struct NetContext *ctx, mbedtls_pk_context *key, uint8_t *in, uint32_t in_len, struct ByteArrayNetSerializable *out);
_Bool MasterServerSession_set_clientPublicKey(struct MasterServerSession *session, struct NetContext *ctx, struct ByteArrayNetSerializable *in);
void MasterServerSession_set_epoch(struct MasterServerSession *session, uint32_t epoch);
_Bool MasterServerSession_set_state(struct MasterServerSession *session, HandshakeMessageType state);
uint32_t *MasterServerSession_ClientHelloWithCookieRequest_requestId(struct MasterServerSession *session);

_Bool net_init(struct NetContext *ctx, uint16_t port);
void net_stop(struct NetContext *ctx);
void net_cleanup(struct NetContext *ctx);
const char *net_tostr(struct SS *a);
uint32_t net_recv(struct NetContext *ctx, struct MasterServerSession **session, PacketProperty *property, uint8_t **buf);
void net_send(struct NetContext *ctx, struct MasterServerSession *session, PacketProperty property, uint8_t *buf, uint32_t len, _Bool reliable);
_Bool net_handle_ack(struct MasterServerSession *session, struct MessageHeader *message_out, struct SerializeHeader *serial_out, uint32_t requestId);
uint32_t net_getNextRequestId(struct MasterServerSession *session);

_Bool status_init(const char *path, uint16_t port);
void status_cleanup();

_Bool status_ssl_init(mbedtls_x509_crt *cert, mbedtls_pk_context *key, const char *path, uint16_t port);
void status_ssl_cleanup();

_Bool master_init(mbedtls_x509_crt *cert, mbedtls_pk_context *key, uint16_t port);
void master_cleanup();
