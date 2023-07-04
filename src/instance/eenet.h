#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct _ENetPacket ENetPacket;
struct EENetPacket {
	enum EENetPacketType {
		EENetPacketType_None,
		EENetPacketType_Connect,
		EENetPacketType_Disconnect,
		EENetPacketType_ConnectMessage,
		EENetPacketType_Message,

		EENetPacketType_Reliable = EENetPacketType_Message + 0,
		EENetPacketType_Unreliable = EENetPacketType_Message + 1,
	} type;
	uint32_t data_len;
	const uint8_t *data;
	ENetPacket *ref;
};

typedef struct _ENetHost ENetHost;
struct NetContext;
struct NetSession;
ENetHost *eenet_init();
void eenet_free(ENetHost *host);
void eenet_attach(ENetHost *host, struct NetContext *ctx, struct NetSession *session);
bool eenet_handle(ENetHost *host, const uint8_t *data, const uint8_t *end, struct EENetPacket *pkt_out);
bool eenet_handle_next(ENetHost *host, struct EENetPacket *pkt_out);
void eenet_send(ENetHost *host, const uint8_t *resp, uint32_t resp_len, bool reliable);
void eenet_tick(ENetHost *host);
