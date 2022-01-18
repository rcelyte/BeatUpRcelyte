// #define THREAD_COUNT 256
#define THREAD_COUNT 1

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
typedef uint8_t ClientState;
enum ClientState {
	ClientState_disconnected,
	ClientState_accepted,
	ClientState_connected,
};
struct InstanceSession {
	struct NetSession net;
	struct InstanceSession *next;
	ClientState clientState;
	struct String userId;
	struct PlayerStateHash state;
	struct MultiplayerAvatarData avatar;
	struct Pong pong;
	struct ReliableUnorderedChannel ruChannel;
	struct ReliableOrderedChannel roChannel;
	struct SequencedChannel rsChannel;
};

struct Room {
	struct NetKeypair keys;
	float syncBase;
	uint8_t playerCount, playerLimit, *playerSort;
	struct InstanceSession *players;
};

struct Context {
	struct NetContext net;
	struct Room TEMPglobalRoom; // TODO: room allocation (interface for wire.c)
};

static struct NetSession *instance_onResolve(struct Context *ctx, struct SS addr, void **userdata_out) {
	struct Room *room = &ctx->TEMPglobalRoom;
	for(uint32_t i = 0; i < room->playerCount; ++i) {
		if(addrs_are_equal(&addr, NetSession_get_addr(&room->players[room->playerSort[i]].net))) {
			*userdata_out = room;
			return &room->players[room->playerSort[i]].net;
		}
	}
	return NULL;
}

static void swap(uint8_t *a, uint8_t *b) {
	uint8_t c = *a;
	*a = *b, *b = c;
}

static void room_disconnect(struct Room *room, uint32_t sort) {
	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(NetSession_get_addr(&room->players[room->playerSort[sort]].net), addrstr);
	fprintf(stderr, "[INSTANCE] disconnect %s\n", addrstr);
	net_session_free(&room->players[room->playerSort[sort]].net);
	room->players[room->playerSort[sort]].clientState = ClientState_disconnected;
	swap(&room->playerSort[sort], &room->playerSort[--room->playerCount]);
	fprintf(stderr, "[INSTANCE] player count: %hhu / %hhu\n", room->playerCount, room->playerLimit);
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
	struct Room *room = &ctx->TEMPglobalRoom;
	for(uint32_t i = 0; i < room->playerCount;) {
		struct InstanceSession *session = &room->players[room->playerSort[i]];
		uint32_t kickTime = NetSession_get_lastKeepAlive(&session->net) + 180000;
		if(currentTime > kickTime) {
			room_disconnect(room, i);
		} else {
			if(kickTime < *nextTick)
				*nextTick = kickTime;
			flush_ack(ctx, session, &session->ruChannel.base.ack);
			flush_ack(ctx, session, &session->roChannel.base.ack);
			for(uint_fast8_t i = 0; i < 64; ++i)
				try_resend(ctx, session, &session->ruChannel.base.resend[i], currentTime);
			for(uint_fast8_t i = 0; i < 64; ++i)
				try_resend(ctx, session, &session->roChannel.base.resend[i], currentTime);
			try_resend(ctx, session, &session->rsChannel.resend, currentTime);
			net_flush_merged(&ctx->net, &session->net);
			++i;
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

#define SERIALIZE_MENURPC(pkt, dtype, data) { \
	SERIALIZE_CUSTOM(pkt, InternalMessageType_MultiplayerSession) { \
		pkt_writeMultiplayerSessionMessageHeader(pkt, (struct MultiplayerSessionMessageHeader){ \
			.type = MultiplayerSessionMessageType_MenuRpc, \
		}); \
		pkt_writeMenuRpcHeader(pkt, (struct MenuRpcHeader){ \
			.type = MenuRpcType_##dtype, \
		}); \
		pkt_write##dtype(pkt, data); \
	} \
}

static float room_get_syncTime(struct Room *room) {
	struct timespec now;
	if(clock_gettime(CLOCK_MONOTONIC, &now))
		return 0;
	return now.tv_sec + (now.tv_nsec / 1000) / 1000000.f - room->syncBase;
}

static void handle_MenuRpc(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct MenuRpcHeader rpc = pkt_readMenuRpcHeader(data);
	switch(rpc.type) {
		// default: fprintf(stderr, "BAD MENU RPC TYPE\n");
		default: fprintf(stderr, "[INSTANCE] MenuRpcType not implemented\n"); break;
	}
}

static void handle_PlayerIdentity(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct PlayerIdentity identity = pkt_readPlayerIdentity(data);
	session->state = identity.playerState;
	session->avatar = identity.playerAvatar;
	fprintf(stderr, "TODO: send to other players\n");
	if(session->clientState != ClientState_accepted)
		return;
	session->clientState = ClientState_connected;

	uint8_t resp[65536], *resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
	struct PlayerSortOrderUpdate r_sort;
	r_sort.userId = session->userId;
	r_sort.sortIndex = (session - room->players) / sizeof(*session);
	SERIALIZE_CUSTOM(&resp_end, InternalMessageType_PlayerSortOrderUpdate)
		pkt_writePlayerSortOrderUpdate(&resp_end, r_sort);
	instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
	struct RemoteProcedureCall base = {
		.syncTime = room_get_syncTime(room)
	};
	struct SetIsStartButtonEnabled r_rpc0 = {
		.base = base,
		.reason = CannotStartGameReason_AllPlayersNotInLobby,
	};
	struct GetRecommendedBeatmap r_rpc1 = {
		.base = base,
	};
	struct GetRecommendedGameplayModifiers r_rpc2 = {
		.base = base,
	};
	struct GetIsReady r_rpc3 = {
		.base = base,
	};
	struct GetIsInLobby r_rpc4 = {
		.base = base,
	};
	r_sort.userId = session->userId;
	r_sort.sortIndex = (session - room->players) / sizeof(*session);
	SERIALIZE_MENURPC(&resp_end, SetIsStartButtonEnabled, r_rpc0);
	SERIALIZE_MENURPC(&resp_end, GetRecommendedBeatmap, r_rpc1);
	SERIALIZE_MENURPC(&resp_end, GetRecommendedGameplayModifiers, r_rpc2);
	SERIALIZE_MENURPC(&resp_end, GetIsReady, r_rpc3);
	SERIALIZE_MENURPC(&resp_end, GetIsInLobby, r_rpc4);
	instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
}

static void handle_MultiplayerSession(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct MultiplayerSessionMessageHeader message = pkt_readMultiplayerSessionMessageHeader(data);
	switch(message.type) {
		case MultiplayerSessionMessageType_MenuRpc: handle_MenuRpc(ctx, room, session, data); break;
		case MultiplayerSessionMessageType_GameplayRpc: fprintf(stderr, "[INSTANCE] MultiplayerSessionMessageType_GameplayRpc not implemented\n"); break;
		case MultiplayerSessionMessageType_NodePoseSyncState: fprintf(stderr, "[INSTANCE] MultiplayerSessionMessageType_NodePoseSyncState not implemented\n"); break;
		case MultiplayerSessionMessageType_ScoreSyncState: fprintf(stderr, "[INSTANCE] MultiplayerSessionMessageType_ScoreSyncState not implemented\n"); break;
		case MultiplayerSessionMessageType_NodePoseSyncStateDelta: fprintf(stderr, "[INSTANCE] MultiplayerSessionMessageType_NodePoseSyncStateDelta not implemented\n"); break;
		case MultiplayerSessionMessageType_ScoreSyncStateDelta: fprintf(stderr, "[INSTANCE] MultiplayerSessionMessageType_ScoreSyncStateDelta not implemented\n"); break;
		default: fprintf(stderr, "[INSTANCE] BAD MULTIPLAYER SESSION MESSAGE TYPE\n");
	}
}

static void handle_Unreliable(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	pkt_readRoutingHeader(data);
	struct SerializeHeader serial = pkt_readSerializeHeader(data);
	switch(serial.type) {
		case InternalMessageType_SyncTime: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_SyncTime not implemented\n"); break;
		case InternalMessageType_PlayerConnected: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_PlayerConnected not implemented\n"); break;
		case InternalMessageType_PlayerIdentity: fprintf(stderr, "[INSTANCE] BAD TYPE: InternalMessageType_PlayerConnected\n"); break;
		case InternalMessageType_PlayerLatencyUpdate: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_PlayerLatencyUpdate not implemented\n"); break;
		case InternalMessageType_PlayerDisconnected: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_PlayerDisconnected not implemented\n"); break;
		case InternalMessageType_PlayerSortOrderUpdate: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_PlayerSortOrderUpdate not implemented\n"); break;
		case InternalMessageType_Party: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_Party not implemented\n"); break;
		case InternalMessageType_MultiplayerSession: handle_MultiplayerSession(ctx, room, session, data); break;
		case InternalMessageType_KickPlayer: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_KickPlayer not implemented\n"); break;
		case InternalMessageType_PlayerStateUpdate: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_PlayerStateUpdate not implemented\n"); break;
		case InternalMessageType_PlayerAvatarUpdate: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_PlayerAvatarUpdate not implemented\n"); break;
		default: fprintf(stderr, "[INSTANCE] UNRELIABLE : BAD INTERNAL MESSAGE TYPE\n");
	}
}

static void process_Channeled(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	pkt_readRoutingHeader(data);
	struct SerializeHeader serial = pkt_readSerializeHeader(data);
	switch(serial.type) {
		case InternalMessageType_SyncTime: fprintf(stderr, "[INSTANCE] InternalMessageType_SyncTime not implemented\n"); break;
		case InternalMessageType_PlayerConnected: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerConnected not implemented\n"); break;
		case InternalMessageType_PlayerIdentity: handle_PlayerIdentity(ctx, room, session, data); break;
		case InternalMessageType_PlayerLatencyUpdate: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerLatencyUpdate not implemented\n"); break;
		case InternalMessageType_PlayerDisconnected: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerDisconnected not implemented\n"); break;
		case InternalMessageType_PlayerSortOrderUpdate: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerSortOrderUpdate not implemented\n"); break;
		case InternalMessageType_Party: fprintf(stderr, "[INSTANCE] InternalMessageType_Party not implemented\n"); break;
		case InternalMessageType_MultiplayerSession: handle_MultiplayerSession(ctx, room, session, data); break;
		case InternalMessageType_KickPlayer: fprintf(stderr, "[INSTANCE] InternalMessageType_KickPlayer not implemented\n"); break;
		case InternalMessageType_PlayerStateUpdate: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerStateUpdate not implemented\n"); break;
		case InternalMessageType_PlayerAvatarUpdate: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerAvatarUpdate not implemented\n"); break;
		default: fprintf(stderr, "[INSTANCE] BAD INTERNAL MESSAGE TYPE\n");
	}
}
static void handle_Channeled(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end) {
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
				process_Channeled(ctx, room, session, data);
				channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
				if(channeled.channelId == DeliveryMethod_ReliableOrdered) {
					while(session->roChannel.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE].len) {
						const uint8_t *pkt = session->roChannel.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE].data, *pkt_it = pkt;
						const uint8_t *pkt_end = &pkt[session->roChannel.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE].len];
						session->roChannel.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE].len = 0;
						process_Channeled(ctx, room, session, &pkt);
						channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
						if(pkt_it != pkt_end)
							fprintf(stderr, "[INSTANCE] BAD PACKET LENGTH (expected %zu, read %zu)\n", pkt_end - pkt, pkt_it - pkt);
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
				process_Channeled(ctx, room, session, data);
			}
		}
		case DeliveryMethod_Sequenced: return;
		case DeliveryMethod_ReliableSequenced: {
			int32_t relative = RelativeSequenceNumber(channeled.sequence, session->rsChannel.ack.sequence);
			if(channeled.sequence < NET_MAX_SEQUENCE && relative > 0) {
				session->rsChannel.ack.sequence = channeled.sequence;
				process_Channeled(ctx, room, session, data);
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
static void handle_Ack(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
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
static void handle_Ping(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
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
static void handle_ConnectRequest(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct ConnectRequest req = pkt_readConnectRequest(data);
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_ConnectAccept, 0, 0});
	pkt_writeConnectAccept(&resp_end, (struct ConnectAccept){
		.connectId = req.connectId,
		.connectNum = 0,
		.reusedPeer = 0,
	});
	net_send_internal(&ctx->net, &session->net, resp, resp_end - resp, 1);

	if(session->clientState != ClientState_disconnected)
		return;
	session->clientState = ClientState_accepted;
	session->userId = req.userId;

	resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
	struct SyncTime r_sync;
	r_sync.syncTime = room_get_syncTime(room);
	SERIALIZE_CUSTOM(&resp_end, InternalMessageType_SyncTime)
		pkt_writeSyncTime(&resp_end, r_sync);
	instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
	struct PlayerIdentity r_identity;
	r_identity.playerState.bloomFilter._d0 = 288266110296588352;
	r_identity.playerState.bloomFilter._d1 = 576531121051926529;
	memset(&r_identity.playerAvatar, 0, sizeof(r_identity.playerAvatar));
	r_identity.random.length = 32;
	memcpy(r_identity.random.data, NetKeypair_get_random(&room->keys), 32);
	r_identity.publicEncryptionKey.length = sizeof(r_identity.publicEncryptionKey.data);
	NetKeypair_write_key(&room->keys, &ctx->net, r_identity.publicEncryptionKey.data, &r_identity.publicEncryptionKey.length);
	SERIALIZE_CUSTOM(&resp_end, InternalMessageType_PlayerIdentity)
		pkt_writePlayerIdentity(&resp_end, r_identity);
	instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
}
static void handle_Disconnect(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	pkt_readDisconnect(data);
	for(uint32_t i = 0; i < room->playerCount;) {
		if(&room->players[room->playerSort[i]] == session) {
			room_disconnect(room, i);
			return;
		}
	}
}
static void handle_MtuCheck(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct MtuCheck req = pkt_readMtuCheck(data);
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_MtuOk, 0, 0});
	pkt_writeMtuOk(&resp_end, (struct MtuOk){
		.newMtu0 = req.newMtu0,
		.newMtu1 = req.newMtu1,
	});
	net_send_internal(&ctx->net, &session->net, resp, resp_end - resp, 1);
}

static void handle_packet(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t *pkt, uint32_t len) {
	const uint8_t *data = pkt, *end = &pkt[len];
	struct NetPacketHeader header = pkt_readNetPacketHeader(&data);
	switch(header.property) {
		case PacketProperty_Unreliable: handle_Unreliable(ctx, room, session, &data); break;
		case PacketProperty_Channeled: handle_Channeled(ctx, room, session, &data, end); break;
		case PacketProperty_Ack: handle_Ack(ctx, room, session, &data); break;
		case PacketProperty_Ping: handle_Ping(ctx, room, session, &data); break;
		case PacketProperty_Pong: fprintf(stderr, "[INSTANCE] PacketProperty_Pong not implemented\n"); break;
		case PacketProperty_ConnectRequest: handle_ConnectRequest(ctx, room, session, &data); break;
		case PacketProperty_ConnectAccept: fprintf(stderr, "[INSTANCE] BAD PROPERTY: PacketProperty_ConnectAccept\n"); break;
		case PacketProperty_Disconnect: handle_Disconnect(ctx, room, session, &data); break;
		case PacketProperty_UnconnectedMessage: fprintf(stderr, "[INSTANCE] BAD PROPERTY: PacketProperty_UnconnectedMessage\n"); break;
		case PacketProperty_MtuCheck: handle_MtuCheck(ctx, room, session, &data); break;
		case PacketProperty_MtuOk: fprintf(stderr, "[INSTANCE] PacketProperty_MtuOk not implemented\n"); break;
		case PacketProperty_Broadcast: fprintf(stderr, "[INSTANCE] PacketProperty_Broadcast not implemented\n"); break;
		case PacketProperty_Merged: {
			for(uint16_t len; data < end; data += len) {
				len = pkt_readUint16(&data);
				const uint8_t *sub = data;
				handle_packet(ctx, room, session, sub, len);
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
		fprintf(stderr, "[INSTANCE] BAD PACKET LENGTH (expected %u, read %zu)\n", len, data - pkt);
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
	struct Room *room;
	struct InstanceSession *session;
	const uint8_t *pkt;
	while((len = net_recv(&ctx->net, buf, sizeof(buf), (struct NetSession**)&session, &pkt, (void**)&room))) { // TODO: close instances with zero sessions using `onDisconnect`
		handle_packet(ctx, room, session, pkt, len);
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

		if(net_keypair_init(&ctx[i].net, &ctx[i].TEMPglobalRoom.keys))
			return 1;
		ctx[i].TEMPglobalRoom.syncBase = 0;
		ctx[i].TEMPglobalRoom.syncBase = room_get_syncTime(&ctx[i].TEMPglobalRoom);
		ctx[i].TEMPglobalRoom.playerCount = 0;
		ctx[i].TEMPglobalRoom.playerLimit = 5;
		size_t sortLen = (ctx[i].TEMPglobalRoom.playerLimit * sizeof(*ctx[i].TEMPglobalRoom.playerSort) + 7) & ~7; // aligned
		ctx[i].TEMPglobalRoom.playerSort = malloc(sortLen + ctx[i].TEMPglobalRoom.playerLimit * sizeof(*ctx[i].TEMPglobalRoom.players));
		ctx[i].TEMPglobalRoom.players = (struct InstanceSession*)&((uint8_t*)ctx[i].TEMPglobalRoom.playerSort)[sortLen];
		for(uint32_t i = 0; i < ctx[i].TEMPglobalRoom.playerLimit; ++i) {
			ctx[i].TEMPglobalRoom.playerSort[i] = i;
			ctx[i].TEMPglobalRoom.players[i].clientState = ClientState_disconnected;
		}

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
			while(ctx[i].TEMPglobalRoom.playerCount)
				room_disconnect(&ctx[i].TEMPglobalRoom, ctx[i].TEMPglobalRoom.playerSort[0]);
			net_cleanup(&ctx[i].net);
		}
	}
}

static inline struct Context *ServerCode_get_Context(ServerCode code) {
	return &ctx[(uint64_t)code * (uint64_t)THREAD_COUNT / (uint64_t)62193781];
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
	struct InstanceSession *session = (struct InstanceSession*)instance_onResolve(ctx, addr, (void*[]){NULL}); // TODO: This may connect the user to previous rooms they haven't successfully disconnected from
	if(session) {
		if(net_session_reset(&ctx->net, &session->net))
			return NULL;
	} else {
		if(ctx->TEMPglobalRoom.playerCount >= ctx->TEMPglobalRoom.playerLimit) {
			fprintf(stderr, "ROOM FULL\n");
			return NULL;
		}
		session = &ctx->TEMPglobalRoom.players[ctx->TEMPglobalRoom.playerSort[ctx->TEMPglobalRoom.playerCount]];
		if(net_session_init(&ctx->net, &session->net, addr))
			return NULL;
		++ctx->TEMPglobalRoom.playerCount;
	}
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
	fprintf(stderr, "[INSTANCE] player count: %hhu / %hhu\n", ctx->TEMPglobalRoom.playerCount, ctx->TEMPglobalRoom.playerLimit);
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
