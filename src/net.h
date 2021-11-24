#include "packets.h"
#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>
#include <mbedtls/ctr_drbg.h>

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
ENUM(uint8_t, MasterServerSessionState, {
	MasterServerSessionState_None,
	MasterServerSessionState_New,
	MasterServerSessionState_Established,
	MasterServerSessionState_Authenticated,
})
ENUM(uint8_t, Platform, {
	Test,
	Oculus,
	OculusQuest,
	Steam,
	PS4,
})

struct MasterServerSession;
uint8_t *MasterServerSession_get_clientRandom(struct MasterServerSession *session);
uint8_t *MasterServerSession_get_serverRandom(struct MasterServerSession *session);
uint8_t *MasterServerSession_get_cookie(struct MasterServerSession *session);
_Bool MasterServerSession_write_key(struct MasterServerSession *session, struct ByteArrayNetSerializable *out);
void MasterServerSession_set_epoch(struct MasterServerSession *session, uint32_t epoch);
void MasterServerSession_set_state(struct MasterServerSession *session, MasterServerSessionState state);

const char *net_tostr(struct SS *a);
int32_t net_init(uint16_t port);
void net_cleanup();
uint32_t net_recv(int32_t sockfd, mbedtls_ctr_drbg_context *ctr_drbg, struct MasterServerSession **session, PacketType *property, uint8_t **buf);
void net_send(int32_t sockfd, struct MasterServerSession *session, PacketType property, uint8_t *buf, uint32_t len, _Bool reliable);
uint8_t *net_handle_ack(struct MasterServerSession *session, uint32_t requestId);
uint32_t net_getNextRequestId(struct MasterServerSession *session);

_Bool status_init();
void status_cleanup();

_Bool status_ssl_init(mbedtls_x509_crt *cert, mbedtls_pk_context *key, uint16_t port);
void status_ssl_cleanup();

_Bool master_init(mbedtls_x509_crt *cert, uint16_t port);
void master_cleanup();
