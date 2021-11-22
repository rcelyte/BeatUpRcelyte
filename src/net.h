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
struct MasterServerSession {
	struct SS addr;
	uint32_t epoch;
	uint32_t lastSentRequestId;
	MasterServerSessionState state;
	mbedtls_ecp_keypair key;
	uint8_t cookie[32];
	uint8_t clientRandom[32];
	uint8_t serverRandom[32];
	time_t lastKeepAlive;
};

const char *net_tostr(struct SS *a);
int32_t net_init();
void net_cleanup();
uint32_t net_recv(int32_t sockfd, mbedtls_ctr_drbg_context *ctr_drbg, struct MasterServerSession **session, PacketProperty *property, uint8_t **buf);
void net_send(int32_t sockfd, struct MasterServerSession *session, PacketProperty property, uint8_t *buf, uint32_t len);
uint32_t net_getNextRequestId(struct MasterServerSession *session);

_Bool status_init();
void status_cleanup();

_Bool status_ssl_init(mbedtls_x509_crt srvcert, mbedtls_pk_context pkey);
void status_ssl_cleanup();

_Bool master_init(mbedtls_x509_crt *cert);
void master_cleanup();
