#define ENET_IMPLEMENTATION
#ifndef _WIN32
#include <netdb.h>
#define getaddrinfo getaddrinfo_stub_

// stop enet from linking `getaddrinfo()` to make glibc happy
static inline int getaddrinfo(const char*, const char*, const void*, void*) {
	return -1;
}
#endif
#include "eenet.h"
#include "../net.h"
#include <enet.h>

_Static_assert(EENET_CONNECT_BYTE == (ENET_PROTOCOL_HEADER_FLAG_SENT_TIME | ENET_PROTOCOL_MAXIMUM_PEER_ID) >> 8, "ABI break");

ENetHost *eenet_init() {
	ENetHost *const host = enet_host_create(NULL, 1, 2, 0, 0, 0);
	host->randomSeed = 0;
	return host;
}

void eenet_free(ENetHost *host) {
	if(host != NULL)
		enet_host_destroy(host);
}

struct FakeSocket {
	struct NetContext *ctx;
	struct NetSession *session;
};

static struct EENetPacket EENetPacket_init(const ENetEvent *event) {
	struct EENetPacket out = {
		.type = EENetPacketType_None,
		.ref = event->packet,
	};
	switch(event->type) {
		case ENET_EVENT_TYPE_CONNECT: out.type = EENetPacketType_Connect; break;
		case ENET_EVENT_TYPE_DISCONNECT: out.type = EENetPacketType_Disconnect; break;
		case ENET_EVENT_TYPE_RECEIVE: out.type = EENetPacketType_Message + event->channelID; break;
		case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: out.type = EENetPacketType_Disconnect; break;
		default:;
	}
	if(event->packet != NULL) {
		out.data_len = event->packet->dataLength;
		out.data = event->packet->data;
	}
	return out;
}

bool eenet_handle_next(ENetHost *host, struct EENetPacket *pkt) {
	if(pkt->ref != NULL)
		enet_packet_destroy(pkt->ref);
	*pkt = (struct EENetPacket){0};
	ENetEvent event = {0};
	if(enet_host_check_events(host, &event) <= 0)
		return false;
	*pkt = EENetPacket_init(&event);
	return true;
}

void eenet_attach(ENetHost *host, struct NetContext *ctx, struct NetSession *session) {
	memcpy(host->receivedAddress.ipv6.s6_addr, &(const struct FakeSocket){ctx, session}, sizeof(struct FakeSocket));
	host->peers[0].address = host->receivedAddress;
}

bool eenet_handle(ENetHost *host, const uint8_t *data, const uint8_t *end, struct EENetPacket *pkt_out) {
	*pkt_out = (struct EENetPacket){0};

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wcast-qual"
	host->receivedData = (uint8_t*)data; // TODO: do not.
	#pragma GCC diagnostic pop
	host->receivedDataLength = (size_t)(end - data);
	host->totalReceivedData += host->receivedDataLength;
	++host->totalReceivedPackets;

	ENetEvent event = {0};
	if(enet_protocol_handle_incoming_commands(host, &event) <= 0)
		return eenet_handle_next(host, pkt_out);
	*pkt_out = EENetPacket_init(&event);
	return true;
}

void eenet_send(ENetHost *host, const uint8_t *resp, uint32_t resp_len, bool reliable) {
	enet_peer_send(&host->peers[0], reliable ? 0 : 1, enet_packet_create(resp, resp_len, reliable ? ENET_PACKET_FLAG_RELIABLE : 0));
}

void eenet_tick(ENetHost *host) {
	enet_host_service(host, NULL, 0);
}

int enet_initialize(void) {return 0;}
void enet_deinitialize(void) {}
ENetSocket enet_socket_create(ENetSocketType) {return 0;}
int enet_socket_bind(ENetSocket, const ENetAddress*) {return 0;}
int enet_socket_get_address(ENetSocket, ENetAddress*) {return -1;}
// int enet_socket_listen(ENetSocket, int);
// ENetSocket enet_socket_accept(ENetSocket, ENetAddress*);
// int enet_socket_connect(ENetSocket, const ENetAddress*);
int enet_socket_send(ENetSocket, const ENetAddress *ptr, const ENetBuffer *buffers, size_t buffers_len) {
	struct FakeSocket state = {0};
	memcpy(&state, ptr, sizeof(state));
	if(state.ctx == NULL || state.session == NULL)
		return -1;
	uint32_t body_len = 0;
	uint8_t body[ENET_PROTOCOL_MAXIMUM_MTU];
	for(const ENetBuffer *buffer_it = buffers; buffer_it < &buffers[buffers_len]; ++buffer_it) {
		if(buffer_it->dataLength > sizeof(body) - body_len)
			return -1;
		memcpy(&body[body_len], buffer_it->data, buffer_it->dataLength);
		body_len += buffer_it->dataLength;
	}
	net_send_internal(state.ctx, state.session, body, body_len, EncryptMode_DTLS);
	return (int)body_len;
}
int enet_socket_receive(ENetSocket, ENetAddress*, ENetBuffer*, size_t) {return 0;}
int enet_socket_wait(ENetSocket, uint32_t*, uint64_t) {return -1;}
int enet_socket_set_option(ENetSocket, ENetSocketOption, int) {return 0;}
// int enet_socket_get_option(ENetSocket, ENetSocketOption, int*);
// int enet_socket_shutdown(ENetSocket, ENetSocketShutdown);
void enet_socket_destroy(ENetSocket) {}
// int enet_socket_set_select(ENetSocket, ENetSocketSet*, ENetSocketSet*, uint32_t);
uint64_t enet_host_random_seed(void) {return 0;}
