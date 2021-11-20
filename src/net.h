#include "packets.h"
#include <arpa/inet.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>

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
_Bool net_init(mbedtls_x509_crt srvcert, mbedtls_pk_context pkey);
void net_cleanup();
uint32_t net_recv(struct MasterServerSession **session, PacketProperty *property, uint8_t **buf);
void net_send(struct MasterServerSession *session, PacketProperty property, uint8_t *buf, uint32_t len);
void net_cookie(uint8_t *out);
uint32_t net_getNextRequestId(struct MasterServerSession *session);
