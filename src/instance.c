#define THREAD_COUNT 256
#define ROOM_COUNT 62193781

#include "enum_reflection.h"
#include "instance.h"
#ifdef WINDOWS
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif
#include "debug.h"
#include <mbedtls/error.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define lengthof(x) (sizeof(x)/sizeof(*x))

struct Room {
	_Bool isOpen;
};

struct InstanceResendPacket {
	PacketProperty property;
	uint32_t len;
	uint8_t data[1432];
};
struct InstanceSession {
	struct NetSession net;
	struct InstanceSession *next;
	struct Room *room;
	struct Pong pong;
	uint16_t localWindowStart, localSeqence;
	uint32_t resend_timeStamp[NET_WINDOW_SIZE];
	struct InstanceResendPacket resend_data[NET_WINDOW_SIZE];
};

struct Context {
	struct InstanceSession *sessionList;
	struct NetContext net;
};

static struct NetSession *instance_onResolve(struct Context *ctx, struct SS addr) {
	struct InstanceSession *session = ctx->sessionList;
	for(; session; session = session->next)
		if(addrs_are_equal(&addr, NetSession_get_addr(&session->net)))
			return &session->net;
	return NULL;
}

static struct InstanceSession *instance_disconnect(struct InstanceSession *session) {
	struct InstanceSession *next = session->next;
	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(NetSession_get_addr(&session->net), addrstr);
	fprintf(stderr, "[INSTANCE] disconnect %s\n", addrstr);
	net_session_free(&session->net);
	free(session);
	return next;
}

static void instance_onResend(struct Context *ctx, uint32_t currentTime, uint32_t *nextTick) {
	for(struct InstanceSession **sp = &ctx->sessionList; *sp;) {
		uint32_t kickTime = NetSession_get_lastKeepAlive(&(*sp)->net) + 180000;
		if(currentTime > kickTime) {
			*sp = instance_disconnect(*sp);
		} else {
			if(kickTime < *nextTick)
				*nextTick = kickTime;
			sp = &(*sp)->next;
		}
	}
}

static void handle_Ack(struct Context *ctx, struct InstanceSession *session, const uint8_t **data) {
	struct Ack ack = pkt_readAck(data);
	/*if(packet.Size != _outgoingAcks.Size) // [PA]Invalid acks packet size
		return;*/
	int32_t windowRel = RelativeSequenceNumber(session->localWindowStart, ack.sequence);
	if(ack.sequence >= NET_MAX_SEQUENCE || windowRel < 0 || windowRel >= NET_WINDOW_SIZE) // [PA]Bad window start
		return;
	for(uint16_t pendingSeq = session->localWindowStart; pendingSeq != session->localSeqence; pendingSeq = (pendingSeq + 1) % NET_MAX_SEQUENCE) {
		int32_t rel = RelativeSequenceNumber(pendingSeq, ack.sequence);
		if(rel >= NET_WINDOW_SIZE)
			break;
		uint16_t pendingIdx = pendingSeq % NET_WINDOW_SIZE;
		if((ack.data[pendingIdx / 8] & (1 << (pendingIdx % 8))) == 0) //Skip false ack
			continue;
		if(pendingSeq == session->localWindowStart) //Move window
			session->localWindowStart = (session->localWindowStart + 1) % NET_MAX_SEQUENCE;
		fprintf(stderr, "[INSTANCE] TODO: HANDLE ACK HERE\n");
		/*if(_pendingPackets[pendingIdx].Clear(Peer)) //clear packet
			NetDebug.Write("[PA]Removing reliableInOrder ack: {0} - true", pendingSeq);*/
	}
}
static void handle_Ping(struct Context *ctx, struct InstanceSession *session, const uint8_t **data) {
	struct Ping ping = pkt_readPing(data);
	if(RelativeSequenceNumber(ping.sequence, session->pong.sequence) > 0) {
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		session->pong.sequence = ping.sequence;
		session->pong.time = (uint64_t)now.tv_sec * 10000000LLU + (uint64_t)now.tv_nsec / 100LLU;
	}
}
static void handle_ConnectRequest(struct Context *ctx, struct InstanceSession *session, const uint8_t **data) {
	struct ConnectRequest req = pkt_readConnectRequest(data);
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeConnectAccept(&resp_end, (struct ConnectAccept){
		.connectId = req.connectId,
		.connectNum = 0,
		.reusedPeer = 0,
	});
	net_send(&ctx->net, &session->net, PacketProperty_ConnectAccept, resp, resp_end - resp);
}

static void handle_packet(struct Context *ctx, struct InstanceSession *session, struct NetPacketHeader header, const uint8_t *pkt, uint32_t len) {
	const uint8_t *data = pkt, *end = &pkt[len];
	switch(header.property) {
		case PacketProperty_Unreliable: fprintf(stderr, "[INSTANCE] PacketProperty_Unreliable not implemented\n"); break;
		case PacketProperty_Channeled: {
			pkt_readRoutingHeader(&data);
			struct SerializeHeader serial = pkt_readSerializeHeader(&data);
			switch(serial.type) {
				case InternalMessageType_SyncTime: fprintf(stderr, "[INSTANCE] InternalMessageType_SyncTime not implemented\n"); break;
				case InternalMessageType_PlayerConnected: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerConnected not implemented\n"); break;
				case InternalMessageType_PlayerIdentity: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerIdentity not implemented\n"); break;
				case InternalMessageType_PlayerLatencyUpdate: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerLatencyUpdate not implemented\n"); break;
				case InternalMessageType_PlayerDisconnected: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerDisconnected not implemented\n"); break;
				case InternalMessageType_PlayerSortOrderUpdate: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerSortOrderUpdate not implemented\n"); break;
				case InternalMessageType_Party: fprintf(stderr, "[INSTANCE] InternalMessageType_Party not implemented\n"); break;
				case InternalMessageType_MultiplayerSession: fprintf(stderr, "[INSTANCE] InternalMessageType_MultiplayerSession not implemented\n"); break;
				case InternalMessageType_KickPlayer: fprintf(stderr, "[INSTANCE] InternalMessageType_KickPlayer not implemented\n"); break;
				case InternalMessageType_PlayerStateUpdate: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerStateUpdate not implemented\n"); break;
				case InternalMessageType_PlayerAvatarUpdate: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerAvatarUpdate not implemented\n"); break;
				default: fprintf(stderr, "[INSTANCE] BAD INTERNAL MESSAGE TYPE\n");
			}
		}
		case PacketProperty_Ack: handle_Ack(ctx, session, &data); break;
		case PacketProperty_Ping: handle_Ping(ctx, session, &data); break;
		case PacketProperty_Pong: fprintf(stderr, "[INSTANCE] PacketProperty_Pong not implemented\n"); break;
		case PacketProperty_ConnectRequest: handle_ConnectRequest(ctx, session, &data); break;
		case PacketProperty_ConnectAccept: fprintf(stderr, "[INSTANCE] BAD PROPERTY: PacketProperty_ConnectAccept\n"); break;
		case PacketProperty_Disconnect: fprintf(stderr, "[INSTANCE] PacketProperty_Disconnect not implemented\n"); break;
		case PacketProperty_UnconnectedMessage: fprintf(stderr, "[INSTANCE] BAD PROPERTY: PacketProperty_UnconnectedMessage\n"); break;
		case PacketProperty_MtuCheck: net_send_mtu(&ctx->net, &session->net, PacketProperty_MtuOk, pkt, len); break;
		case PacketProperty_MtuOk: fprintf(stderr, "[INSTANCE] PacketProperty_MtuOk not implemented\n"); break;
		case PacketProperty_Broadcast: fprintf(stderr, "[INSTANCE] PacketProperty_Broadcast not implemented\n"); break;
		case PacketProperty_Merged: {
			for(uint16_t len; data < end; data += len) {
				len = pkt_readUint16(&data);
				const uint8_t *sub = data;
				struct NetPacketHeader subheader = pkt_readNetPacketHeader(&sub);
				if(sub <= &data[len]) {
					fprintf(stderr, "[@%zu]:\n", sub - pkt);
					handle_packet(ctx, session, subheader, sub, len);
				}
			}
			break;
		}
		case PacketProperty_ShutdownOk: fprintf(stderr, "[INSTANCE] PacketProperty_ShutdownOk not implemented\n"); break;
		case PacketProperty_PeerNotFound: fprintf(stderr, "[INSTANCE] PacketProperty_PeerNotFound not implemented\n"); break;
		case PacketProperty_InvalidProtocol: fprintf(stderr, "[INSTANCE] PacketProperty_InvalidProtocol not implemented\n"); break;
		case PacketProperty_NatMessage: fprintf(stderr, "[INSTANCE] PacketProperty_NatMessage not implemented\n"); break;
		case PacketProperty_Empty: fprintf(stderr, "[INSTANCE] PacketProperty_Empty not implemented\n"); break;
		default: fprintf(stderr, "[INSTANCE] BAD PACKET PROPERTY\n");
	}
	if(data != end)
		fprintf(stderr, "[INSTANCE] BAD PACKET LENGTH (expected %u, got %zu)\n", len, data - pkt);
}

#ifdef WINDOWS
static DWORD WINAPI
#else
static void*
#endif
instance_handler(struct Context *ctx) {
	fprintf(stderr, "[INSTANCE] Started\n");
	uint8_t buf[262144];
	memset(buf, 0, sizeof(buf));
	uint32_t len;
	struct InstanceSession *session;
	struct NetPacketHeader header;
	const uint8_t *pkt;
	while((len = net_recv(&ctx->net, buf, sizeof(buf), (struct NetSession**)&session, &header, &pkt))) { // TODO: close instances with zero sessions using `onDisconnect`
		handle_packet(ctx, session, header, pkt, len);
	}
	return 0;
}

#ifdef WINDOWS
static HANDLE instance_threads[THREAD_COUNT];
#else
static pthread_t instance_threads[THREAD_COUNT];
#endif
static struct Context ctx[THREAD_COUNT];
// static uint32_t instance_count = 1; // ServerCode 0 ("") is not valid
static const char *instance_domain = NULL;
_Bool instance_init(const char *domain) {
	instance_domain = domain;
	memset(instance_threads, 0, sizeof(instance_threads));
	for(uint32_t i = 0; i < lengthof(instance_threads); ++i) {
		if(net_init(&ctx[i].net, 0)) {
			fprintf(stderr, "net_init() failed\n");
			return 1;
		}
		ctx[i].net.user = &ctx[i];
		ctx[i].net.onResolve = instance_onResolve;
		ctx[i].net.onResend = instance_onResend;
		#ifdef WINDOWS
		instance_threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)instance_handler, &ctx[i], 0, NULL);
		#else
		if(pthread_create(&instance_threads[i], NULL, (void*(*)(void*))&instance_handler, &ctx[i]))
			instance_threads[i] = 0;
		#endif
		if(!instance_threads[i]) {
			fprintf(stderr, "[INSTANCE] Instance thread creation failed\n");
			return 1;
		}
	}
	return 0;
}

void instance_cleanup() {
	for(uint32_t i = 0; i < lengthof(instance_threads); ++i) {
		if(instance_threads[i]) {
			net_stop(&ctx[i].net);
			fprintf(stderr, "[INSTANCE] Stopping #%u\n", i);
			#ifdef WINDOWS
			WaitForSingleObject(instance_threads[i], INFINITE);
			#else
			pthread_join(instance_threads[i], NULL);
			#endif
			instance_threads[i] = 0;
			while(ctx[i].sessionList)
				ctx[i].sessionList = instance_disconnect(ctx[i].sessionList);
			net_cleanup(&ctx[i].net);
		}
	}
}

static inline struct Context *ServerCode_get_Context(ServerCode code) {
	return &ctx[(uint64_t)code * (uint64_t)THREAD_COUNT / (uint64_t)ROOM_COUNT];
}

_Bool instance_get_isopen(ServerCode code) {
	return code == 1; // TODO: temporary hack
}
_Bool instance_open(ServerCode *out) {
	*out = 1; // TODO: temporary hack
	return 0;
}

struct NetContext *instance_get_net(ServerCode code) {
	return &ServerCode_get_Context(code)->net;
}

struct NetSession *instance_resolve_session(ServerCode code, struct SS addr) {
	struct Context *ctx = ServerCode_get_Context(code);
	struct InstanceSession *session = (struct InstanceSession*)instance_onResolve(ctx, addr);
	if(session) {
		if(net_session_reset(&ctx->net, &session->net))
			return NULL;
	} else {
		session = malloc(sizeof(struct InstanceSession));
		if(net_session_init(&ctx->net, &session->net, addr)) {
			free(session);
			return NULL;
		}
		session->next = ctx->sessionList;
		ctx->sessionList = session;
	}
	if(!session) {
		fprintf(stderr, "alloc error\n");
		return NULL;
	}
	if(net_session_init(&ctx->net, &session->net, addr)) {
		free(session);
		return NULL;
	}
	session->room = NULL;
	session->pong.sequence = 0;
	session->localWindowStart = 0;
	session->localSeqence = 0;

	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(&addr, addrstr);
	fprintf(stderr, "[INSTANCE] connect %s\n", addrstr);
	return &session->net;
}

struct IPEndPoint instance_get_address(ServerCode code) {
	struct IPEndPoint out;
	out.address.length = sprintf(out.address.data, "%s", instance_domain);
	struct SS addr = {sizeof(struct sockaddr_storage)};
	getsockname(net_get_sockfd(&ServerCode_get_Context(code)->net), &addr.sa, &addr.len);
	switch(addr.ss.ss_family) {
		case AF_INET: {
			out.port = htons(addr.in.sin_port);
			break;
		}
		case AF_INET6: {
			out.port = htons(addr.in6.sin6_port);
			break;
		}
		default:
		out.port = 0;
	}
	return out;
}
