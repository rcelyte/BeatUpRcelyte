#pragma once
#include "master.h"
#include "../counter.h"

struct MasterSession {
	struct NetSession net;
	struct MasterSession *next;
	uint32_t epoch;
	uint32_t lastSentRequestId;
	struct HandshakeState {
		uint32_t certificateRequestId;
		uint32_t helloResponseId;
		uint32_t certificateOutboundCount;
		HandshakeMessageType step;
	} handshake;
	struct MasterResend {
		struct Counter64 set;
		struct MasterPacket {
			uint32_t firstSend, lastSend;
			uint16_t length;
			bool encrypt;
			uint8_t data[512];
		} slots[64];
		uint32_t requestIds[64];
	} resend;
	struct MasterMultipartList {
		struct MasterMultipartList *next;
		uint32_t id;
		uint32_t totalLength;
		uint16_t count;
		uint8_t data[];
	} *multipartList;
	ENetHost *enet;
};

uint32_t MasterSession_nextRequestId(struct MasterSession *session);
uint32_t MasterSession_send(struct NetContext *net, struct MasterSession *session, MessageType type, const void *message);
void LocalMasterContext_process_ConnectToServerRequest(struct NetContext *net, struct MasterSession *session, const struct ConnectToServerRequest *req);

extern struct LocalMasterContext LocalMasterContext_Instance;
