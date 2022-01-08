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

struct InstancePacket {
	uint32_t len;
	uint8_t data[NET_MAX_PKT_SIZE];
};
struct InstanceResendPacket {
	uint32_t timeStamp, len;
	uint8_t data[NET_MAX_PKT_SIZE];
};
struct ReliableChannel {
	struct Ack ack;
	uint16_t localSeqence, remoteSequence;
	uint16_t localWindowStart, remoteWindowStart;
	struct InstanceResendPacket resend[NET_WINDOW_SIZE];
};
struct ReliableUnorderedChannel {
	struct ReliableChannel base;
	_Bool earlyReceived[NET_WINDOW_SIZE];
};
struct ReliableOrderedChannel {
	struct ReliableChannel base;
	struct InstancePacket receivedPackets[NET_WINDOW_SIZE];
};
struct SequencedChannel {
	struct Ack ack;
	uint16_t localSeqence;
	struct InstanceResendPacket resend;
};
struct InstanceSession {
	struct NetSession net;
	struct InstanceSession *next;
	_Bool connected;
	struct PlayerStateHash state;
	struct MultiplayerAvatarData avatar;
	struct Room *room;
	struct Pong pong;
	struct ReliableUnorderedChannel ruChannel;
	struct ReliableOrderedChannel roChannel;
	struct SequencedChannel rsChannel;
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

static void try_resend(struct Context *ctx, struct InstanceSession *session, struct InstanceResendPacket *p, uint32_t currentTime) {
	if(p->len == 0 || currentTime - p->timeStamp < NET_RESEND_DELAY)
		return;
	net_queue_merged(&ctx->net, &session->net, p->data, p->len);
	while(currentTime - p->timeStamp >= NET_RESEND_DELAY)
		p->timeStamp += NET_RESEND_DELAY;
}

static void flush_ack(struct Context *ctx, struct InstanceSession *session, struct Ack *ack) {
	for(uint_fast8_t i = 0; i < lengthof(ack->data); ++i) {
		if(ack->data[i]) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_Ack, 0, 0});
			pkt_writeAck(&resp_end, *ack);
			net_queue_merged(&ctx->net, &session->net, resp, resp_end - resp);
			memset(ack->data, 0, sizeof(ack->data));
			return;
		}
	}
}

static void instance_onResend(struct Context *ctx, uint32_t currentTime, uint32_t *nextTick) {
	for(struct InstanceSession **sp = &ctx->sessionList; *sp;) {
		uint32_t kickTime = NetSession_get_lastKeepAlive(&(*sp)->net) + 180000;
		if(currentTime > kickTime) {
			*sp = instance_disconnect(*sp);
		} else {
			if(kickTime < *nextTick)
				*nextTick = kickTime;
			struct InstanceSession *session = *sp;
			flush_ack(ctx, session, &session->ruChannel.base.ack);
			flush_ack(ctx, session, &session->roChannel.base.ack);
			for(uint_fast8_t i = 0; i < 64; ++i)
				try_resend(ctx, session, &session->ruChannel.base.resend[i], currentTime);
			for(uint_fast8_t i = 0; i < 64; ++i)
				try_resend(ctx, session, &session->roChannel.base.resend[i], currentTime);
			try_resend(ctx, session, &session->rsChannel.resend, currentTime);
			net_flush_merged(&ctx->net, &session->net);
			sp = &(*sp)->next;
		}
	}
}

static void instance_send_channeled(struct NetContext *ctx, struct InstanceSession *session, uint8_t *buf, uint32_t len, DeliveryMethod method) {
	if(method != DeliveryMethod_ReliableOrdered) {
		fprintf(stderr, "instance_send_channeled(DeliveryMethod_%s) not implemented\n", reflect(DeliveryMethod, method));
		abort();
	}
	struct ReliableChannel *channel = &session->roChannel.base;
	if(RelativeSequenceNumber(channel->localSeqence, channel->localWindowStart) >= NET_WINDOW_SIZE) {
		fprintf(stderr, "Resend overflow buffer not implemented\n");
		abort();
	}
	channel->resend[channel->localSeqence % NET_WINDOW_SIZE].timeStamp = net_time() - NET_RESEND_DELAY;
	uint8_t *pkt = channel->resend[channel->localSeqence % NET_WINDOW_SIZE].data, *pkt_end = pkt;
	pkt_writeNetPacketHeader(&pkt_end, (struct NetPacketHeader){PacketProperty_Channeled, 0, 0});
	pkt_writeChanneled(&pkt_end, (struct Channeled){
		.sequence = channel->localSeqence,
		.channelId = method,
	});
	pkt_writeUint8Array(&pkt_end, buf, len);
	channel->resend[channel->localSeqence % NET_WINDOW_SIZE].len = pkt_end - pkt;
	channel->localSeqence = (channel->localSeqence + 1) % NET_MAX_SEQUENCE;
}

static void handle_PlayerIdentity(struct Context *ctx, struct InstanceSession *session, const uint8_t **data) {
	struct PlayerIdentity identity = pkt_readPlayerIdentity(data);
	session->state = identity.playerState;
	session->avatar = identity.playerAvatar;
	fprintf(stderr, "TODO: send to other players\n");
}

static void process_Channeled(struct Context *ctx, struct InstanceSession *session, const uint8_t **data) {
	pkt_readRoutingHeader(data);
	struct SerializeHeader serial = pkt_readSerializeHeader(data);
	switch(serial.type) {
		case InternalMessageType_SyncTime: fprintf(stderr, "[INSTANCE] InternalMessageType_SyncTime not implemented\n"); break;
		case InternalMessageType_PlayerConnected: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerConnected not implemented\n"); break;
		case InternalMessageType_PlayerIdentity: handle_PlayerIdentity(ctx, session, data); break;
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
static void handle_Channeled(struct Context *ctx, struct InstanceSession *session, const uint8_t **data, const uint8_t *end) {
	struct Channeled channeled = pkt_readChanneled(data);
	if(channeled.sequence >= NET_MAX_SEQUENCE)
		return;
	struct ReliableChannel *channel = &session->roChannel.base;
	switch(channeled.channelId) {
		case DeliveryMethod_ReliableUnordered: channel = &session->ruChannel.base;
		case DeliveryMethod_ReliableOrdered: {
			if(channeled.sequence >= NET_MAX_SEQUENCE)
				return;
			int32_t relate = RelativeSequenceNumber(channeled.sequence, channel->remoteWindowStart);
			if(RelativeSequenceNumber(channeled.sequence, channel->remoteSequence) > NET_WINDOW_SIZE)
				return;
			if(relate < 0 || relate >= NET_WINDOW_SIZE * 2)
				return;
			if(relate >= NET_WINDOW_SIZE) {
				uint16_t newWindowStart = (channel->remoteWindowStart + relate - NET_WINDOW_SIZE + 1) % NET_MAX_SEQUENCE;
				channel->ack.sequence = newWindowStart;
				while(channel->remoteWindowStart != newWindowStart) {
					uint16_t ackIdx = channel->remoteWindowStart % NET_WINDOW_SIZE;
					channel->ack.data[ackIdx / 8] &= ~(1 << (ackIdx % 8));
					channel->remoteWindowStart = (channel->remoteWindowStart + 1) % NET_MAX_SEQUENCE;
				}
			}
			uint16_t ackIdx = channeled.sequence % NET_WINDOW_SIZE;
			if(channel->ack.data[ackIdx / 8] & (1 << (ackIdx % 8)))
				return;
			channel->ack.data[ackIdx / 8] |= 1 << (ackIdx % 8);
			if(channeled.sequence == channel->remoteSequence) {
				process_Channeled(ctx, session, data);
				channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
				if(channeled.channelId == DeliveryMethod_ReliableOrdered) {
					while(session->roChannel.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE].len) {
						const uint8_t *pkt = session->roChannel.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE].data, *pkt_it = pkt;
						const uint8_t *pkt_end = &pkt[session->roChannel.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE].len];
						session->roChannel.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE].len = 0;
						process_Channeled(ctx, session, &pkt);
						channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
						if(pkt_it != pkt_end)
							fprintf(stderr, "[INSTANCE] BAD PACKET LENGTH (expected %zu, got %zu)\n", pkt_end - pkt, pkt_it - pkt);
					}
				} else {
					while(session->ruChannel.earlyReceived[channel->remoteSequence % NET_WINDOW_SIZE]) {
						session->ruChannel.earlyReceived[channel->remoteSequence % NET_WINDOW_SIZE] = 0;
						channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
					}
				}
			} else if(channeled.channelId == DeliveryMethod_ReliableOrdered) {
				session->roChannel.receivedPackets[ackIdx].len = end - *data;
				memcpy(session->roChannel.receivedPackets[ackIdx].data, *data, end - *data);
				*data = end;
			} else {
				session->ruChannel.earlyReceived[ackIdx] = 1;
				process_Channeled(ctx, session, data);
			}
		}
		case DeliveryMethod_Sequenced: return;
		case DeliveryMethod_ReliableSequenced: {
			int32_t relative = RelativeSequenceNumber(channeled.sequence, session->rsChannel.ack.sequence);
			if(channeled.sequence < NET_MAX_SEQUENCE && relative > 0) {
				session->rsChannel.ack.sequence = channeled.sequence;
				process_Channeled(ctx, session, data);
			}
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_Ack, 0, 0});
			pkt_writeAck(&resp_end, session->rsChannel.ack);
			net_queue_merged(&ctx->net, &session->net, resp, resp_end - resp);
		}
		default:;
	}
	return;
}
static void handle_Ack(struct Context *ctx, struct InstanceSession *session, const uint8_t **data) {
	struct Ack ack = pkt_readAck(data);
	/*if(packet.Size != _outgoingAcks.Size) // [PA]Invalid acks packet size
		return;*/
	if(ack.channelId == DeliveryMethod_ReliableSequenced) {
		if(ack.sequence == session->rsChannel.localSeqence)
			session->rsChannel.resend.len = 0;
		return;
	}
	struct ReliableChannel *channel = (ack.channelId == DeliveryMethod_ReliableUnordered) ? &session->ruChannel.base : &session->roChannel.base;
	int32_t windowRel = RelativeSequenceNumber(channel->localWindowStart, ack.sequence);
	if(ack.sequence >= NET_MAX_SEQUENCE || windowRel < 0 || windowRel >= NET_WINDOW_SIZE) // [PA]Bad window start
		return;
	for(uint16_t pendingSeq = channel->localWindowStart; pendingSeq != channel->localSeqence; pendingSeq = (pendingSeq + 1) % NET_MAX_SEQUENCE) {
		if(RelativeSequenceNumber(pendingSeq, ack.sequence) >= NET_WINDOW_SIZE)
			break;
		uint16_t pendingIdx = pendingSeq % NET_WINDOW_SIZE;
		if((ack.data[pendingIdx / 8] & (1 << (pendingIdx % 8))) == 0) //Skip false ack
			continue;
		if(pendingSeq == channel->localWindowStart) //Move window
			channel->localWindowStart = (channel->localWindowStart + 1) % NET_MAX_SEQUENCE;
		channel->resend[pendingIdx].len = 0;
	}
}
static void handle_Ping(struct Context *ctx, struct InstanceSession *session, const uint8_t **data) {
	struct Ping ping = pkt_readPing(data);
	if(RelativeSequenceNumber(ping.sequence, session->pong.sequence) > 0) {
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		session->pong.sequence = ping.sequence;
		session->pong.time = (uint64_t)now.tv_sec * 10000000LLU + (uint64_t)now.tv_nsec / 100LLU;
		uint8_t resp[65536], *resp_end = resp;
		pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_Pong, 0, 0});
		pkt_writePong(&resp_end, session->pong);
		net_send_internal(&ctx->net, &session->net, resp, resp_end - resp, 1);
	}
}
static void handle_ConnectRequest(struct Context *ctx, struct InstanceSession *session, const uint8_t **data) {
	struct ConnectRequest req = pkt_readConnectRequest(data);
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_ConnectAccept, 0, 0});
	pkt_writeConnectAccept(&resp_end, (struct ConnectAccept){
		.connectId = req.connectId,
		.connectNum = 0,
		.reusedPeer = 0,
	});
	net_send_internal(&ctx->net, &session->net, resp, resp_end - resp, 1);

	if(session->connected)
		return;
	session->connected = 1;

	resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
	struct SyncTime r_sync;
	r_sync.syncTime = 0;
	fprintf(stderr, "TODO: actually sync time\n");
	SERIALIZE_BODY(&resp_end, InternalMessageType_SyncTime, SyncTime, r_sync);
	instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
	struct PlayerIdentity r_identity;
	r_identity.playerState.bloomFilter._d0 = 288266110296588352;
	r_identity.playerState.bloomFilter._d1 = 576531121051926529;
	memset(&r_identity.playerAvatar, 0, sizeof(r_identity.playerAvatar));
	r_identity.random.length = 0;
	r_identity.publicEncryptionKey.length = 0;
	SERIALIZE_BODY(&resp_end, InternalMessageType_PlayerIdentity, PlayerIdentity, r_identity);
	instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
}
static void handle_Disconnect(struct Context *ctx, struct InstanceSession *session, const uint8_t **data) {
	for(struct InstanceSession **sp = &ctx->sessionList; *sp; *sp = (*sp)->next) {
		if(*sp == session) {
			*sp = instance_disconnect(*sp);
			return;
		}
	}
}
static void handle_MtuCheck(struct Context *ctx, struct InstanceSession *session, const uint8_t **data) {
	struct MtuCheck req = pkt_readMtuCheck(data);
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_MtuOk, 0, 0});
	pkt_writeMtuOk(&resp_end, (struct MtuOk){
		.newMtu0 = req.newMtu0,
		.newMtu1 = req.newMtu1,
	});
	net_send_internal(&ctx->net, &session->net, resp, resp_end - resp, 1);
}

static void handle_packet(struct Context *ctx, struct InstanceSession *session, const uint8_t *pkt, uint32_t len) {
	const uint8_t *data = pkt, *end = &pkt[len];
	struct NetPacketHeader header = pkt_readNetPacketHeader(&data);
	switch(header.property) {
		case PacketProperty_Unreliable: fprintf(stderr, "[INSTANCE] PacketProperty_Unreliable not implemented\n"); break;
		case PacketProperty_Channeled: handle_Channeled(ctx, session, &data, end); break;
		case PacketProperty_Ack: handle_Ack(ctx, session, &data); break;
		case PacketProperty_Ping: handle_Ping(ctx, session, &data); break;
		case PacketProperty_Pong: fprintf(stderr, "[INSTANCE] PacketProperty_Pong not implemented\n"); break;
		case PacketProperty_ConnectRequest: handle_ConnectRequest(ctx, session, &data); break;
		case PacketProperty_ConnectAccept: fprintf(stderr, "[INSTANCE] BAD PROPERTY: PacketProperty_ConnectAccept\n"); break;
		case PacketProperty_Disconnect: handle_Disconnect(ctx, session, &data); break;
		case PacketProperty_UnconnectedMessage: fprintf(stderr, "[INSTANCE] BAD PROPERTY: PacketProperty_UnconnectedMessage\n"); break;
		case PacketProperty_MtuCheck: handle_MtuCheck(ctx, session, &data); break;
		case PacketProperty_MtuOk: fprintf(stderr, "[INSTANCE] PacketProperty_MtuOk not implemented\n"); break;
		case PacketProperty_Broadcast: fprintf(stderr, "[INSTANCE] PacketProperty_Broadcast not implemented\n"); break;
		case PacketProperty_Merged: {
			for(uint16_t len; data < end; data += len) {
				len = pkt_readUint16(&data);
				const uint8_t *sub = data;
				fprintf(stderr, "[@%zu]:\n", sub - pkt);
				handle_packet(ctx, session, sub, len);
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
	const uint8_t *pkt;
	while((len = net_recv(&ctx->net, buf, sizeof(buf), (struct NetSession**)&session, &pkt))) { // TODO: close instances with zero sessions using `onDisconnect`
		handle_packet(ctx, session, pkt, len);
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
	// if(roomCount == StringToServerCode(NULL, 0))
	// 	++roomCount;
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
	session->connected = 0;
	session->room = NULL;
	session->pong.sequence = 0;
	memset(&session->ruChannel, 0, sizeof(session->ruChannel));
	memset(&session->roChannel, 0, sizeof(session->roChannel));
	memset(&session->rsChannel, 0, sizeof(session->rsChannel));
	session->ruChannel.base.ack.channelId = DeliveryMethod_ReliableUnordered;
	session->roChannel.base.ack.channelId = DeliveryMethod_ReliableOrdered;
	session->rsChannel.ack.channelId = DeliveryMethod_ReliableSequenced;

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
