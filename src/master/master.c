#include "../global.h"
#include "master.h"
#include "pool.h"
#include "../counter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MASTER_SERIALIZE(data, pkt, end) pkt_serialize(data, pkt, end, PV_LEGACY_DEFAULT)

struct HandshakeState {
	uint32_t certificateRequestId;
	uint32_t helloResponseId;
	uint32_t certificateOutboundCount;
	HandshakeMessageType step;
};
struct MasterPacket {
	uint32_t firstSend, lastSend;
	uint16_t length;
	bool encrypt;
	uint8_t data[512];
};
struct MasterResend {
	struct Counter64 set;
	struct MasterPacket slots[64];
	uint32_t requestIds[64];
};
struct MasterMultipartList {
	struct MasterMultipartList *next;
	uint32_t id;
	uint32_t totalLength;
	uint16_t count;
	uint8_t data[];
};
struct MasterSession {
	struct NetSession net;
	struct MasterSession *next;
	uint32_t epoch;
	uint32_t lastSentRequestId;
	struct HandshakeState handshake;
	struct MasterResend resend;
	struct MasterMultipartList *multipartList;
};

struct LocalMasterContext {
	struct MasterContext base;
	struct NetContext net;
	struct WireContext wire;
	struct WireLink *status;
};

struct MasterSession *MasterContext_lookup(struct MasterContext *ctx, struct SS addr) {
	for(struct MasterSession *session = ctx->sessionList; session; session = session->next)
		if(SS_equal(&addr, NetSession_get_addr(&session->net)))
			return session;
	return NULL;
}

struct NetSession *MasterContext_onResolve(struct MasterContext *ctx, struct NetContext *net, struct SS addr, const uint8_t packet[static 1536], uint32_t packet_len, uint8_t out[static 1536], uint32_t *out_len) {
	struct MasterSession *session = MasterContext_lookup(ctx, addr);
	if(session != NULL) {
		*out_len = NetSession_decrypt(&session->net, packet, packet_len, out);
		return &session->net;
	}
	if(packet_len && *packet == 1)
		return NULL;
	session = malloc(sizeof(struct MasterSession));
	if(session == NULL) {
		uprintf("alloc error\n");
		return NULL;
	}
	NetSession_init(&session->net, net, addr);
	session->lastSentRequestId = 0;
	session->handshake.step = HandshakeMessageType_ClientHelloRequest;
	session->resend.set = COUNTER64_CLEAR;
	session->multipartList = NULL;
	session->next = ctx->sessionList;
	ctx->sessionList = session;

	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(&addr, addrstr);
	uprintf("connect %s\n", addrstr);

	*out_len = NetSession_decrypt(&session->net, packet, packet_len, out);
	return &session->net;
}

static struct NetSession *master_onResolve(struct NetContext *net, struct SS addr, const uint8_t packet[static 1536], uint32_t packet_len, uint8_t out[static 1536], uint32_t *out_len, void**) {
	return MasterContext_onResolve(&((struct LocalMasterContext*)net->userptr)->base, net, addr, packet, packet_len, out, out_len);
}

static struct MasterSession *master_disconnect(struct MasterSession *session) {
	struct MasterSession *next = session->next;
	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(NetSession_get_addr(&session->net), addrstr);
	uprintf("disconnect %s\n", addrstr);
	while(session->multipartList) {
		struct MasterMultipartList *e = session->multipartList;
		session->multipartList = session->multipartList->next;
		free(e);
	}
	NetSession_free(&session->net);
	free(session);
	return next;
}

uint32_t MasterContext_onResend(struct MasterContext *ctx, struct NetContext *net, uint32_t currentTime) {
	uint32_t nextTick = 180000;
	for(struct MasterSession **sp = &ctx->sessionList; *sp;) {
		uint32_t kickTime = NetSession_get_lastKeepAlive(&(*sp)->net) + 180000 - currentTime;
		if((int32_t)kickTime < 0) { // this filters the RFC-1149 user
			*sp = master_disconnect(*sp);
			continue;
		}
		if(kickTime < nextTick)
			nextTick = kickTime;
		struct MasterSession *session = *sp;
		struct Counter64 iter = session->resend.set;
		bool unresponsive = false;
		for(uint32_t i; (i = Counter64_clear_next(&iter)) != COUNTER64_INVALID;) {
			struct MasterPacket *slot = &session->resend.slots[i];
			if(!slot->length)
				continue;
			if((int32_t)(slot->lastSend - currentTime) <= 0) {
				unresponsive |= (slot->lastSend - slot->firstSend) > IDLE_TIMEOUT_MS;
				net_send_internal(net, &session->net, slot->data, slot->length, slot->encrypt);
				slot->lastSend = currentTime + NET_RESEND_DELAY - (currentTime - slot->lastSend) % NET_RESEND_DELAY;
			}
			if(slot->lastSend - currentTime < nextTick)
				nextTick = slot->lastSend - currentTime;
		}
		if(unresponsive) // avoids spamming resends for the full 3 minute timeout if the client isn't handling them
			*sp = master_disconnect(*sp);
		else
			sp = &(*sp)->next;
	}
	return nextTick;
}

static uint32_t master_onResend(struct NetContext *net, uint32_t currentTime) {
	return MasterContext_onResend(&((struct LocalMasterContext*)net->userptr)->base, net, currentTime);
}

static uint32_t master_getNextRequestId(struct MasterSession *session) {
	++session->lastSentRequestId;
	return (session->lastSentRequestId & 63) | session->epoch;
}

// return is valid until the next call to `master_send()`
static const struct MasterPacket *master_handle_ack(struct MasterSession *session, const struct MessageReceivedAcknowledge *ack) {
	uint32_t requestId = ack->base.responseId;
	struct Counter64 iter = session->resend.set;
	for(uint32_t i; (i = Counter64_clear_next(&iter)) != COUNTER64_INVALID;) {
		if(requestId != session->resend.requestIds[i])
			continue;
		Counter64_clear(&session->resend.set, i);
		return &session->resend.slots[i];
	}
	return NULL;
}

static void master_send_internal(struct NetContext *net, struct MasterSession *session, const uint8_t *buf, uint16_t length, uint32_t requestId, bool encrypt) {
	uint32_t i = Counter64_set_next(&session->resend.set);
	if(i != COUNTER64_INVALID) {
		struct MasterPacket *slot = &session->resend.slots[i];
		slot->firstSend = slot->lastSend = net_time();
		slot->length = length;
		slot->encrypt = encrypt;
		memcpy(slot->data, buf, length);
		session->resend.requestIds[i] = requestId;
	} else {
		uprintf("RESEND BUFFER FULL\n");
		net_send_internal(net, &session->net, buf, length, encrypt);
	}
}

static uint32_t read_requestId(const uint8_t *data, const uint8_t *data_end, struct PacketContext version) {
	struct MasterServerReliableRequestProxy out = {0};
	pkt_read(&(struct SerializeHeader){0}, &data, data_end, version);
	pkt_read(&out, &data, data_end, version);
	return out.value.requestId;
}

static uint32_t master_send(struct NetContext *net, struct MasterSession *session, MessageType type, const uint8_t *data, const uint8_t *data_end, bool reliable) {
	uint8_t buf[data_end + 64 - data], *buf_end = buf;
	struct UnconnectedMessage header = {
		.type = type,
		.protocolVersion = session->net.version.protocolVersion,
	};
	if(data_end - data <= 414) {
		bool res = pkt_write_c(&buf_end, endof(buf), PV_LEGACY_DEFAULT, NetPacketHeader, {
			.property = PacketProperty_UnconnectedMessage,
			.unconnectedMessage = header,
		}) && pkt_write_bytes(data, &buf_end, endof(buf), PV_LEGACY_DEFAULT, (size_t)(data_end - data));
		if(!res)
			return 1;
		if(reliable)
			master_send_internal(net, session, buf, (uint16_t)(buf_end - buf), read_requestId(data, data_end, PV_LEGACY_DEFAULT), type != MessageType_HandshakeMessage);
		else
			net_send_internal(net, &session->net, buf, (uint16_t)(buf_end - buf), type != MessageType_HandshakeMessage);
		return 1;
	}
	if(!(pkt_write(&header, &buf_end, endof(buf), PV_LEGACY_DEFAULT) &&
	     pkt_write_bytes(data, &buf_end, endof(buf), PV_LEGACY_DEFAULT, (size_t)(data_end - data))))
		return 0;
	struct MultipartMessageProxy mpp = {
		.value = {
			.multipartMessageId = read_requestId(data, data_end, PV_LEGACY_DEFAULT),
			.length = 384,
			.totalLength = (uint32_t)(buf_end - buf),
		},
	};
	switch(type) {
		case MessageType_UserMessage: mpp.type = UserMessageType_MultipartMessage; break;
		case MessageType_GameLiftMessage: mpp.type = GameLiftMessageType_MultipartMessage; break;
		case MessageType_HandshakeMessage: mpp.type = HandshakeMessageType_MultipartMessage; break;
		default: uprintf("Bad MessageType in master_send()\n"); return 0;
	}
	const uint8_t *buf_it = buf;
	uint32_t partCount = 0;
	do {
		mpp.value.base.requestId = master_getNextRequestId(session);
		mpp.value.offset = (uint32_t)(buf_it - buf);
		if((uintptr_t)(buf_end - buf_it) < mpp.value.length)
			mpp.value.length = (uint32_t)(buf_end - buf_it);
		uint8_t mpbuf[512], *mpbuf_end = mpbuf;
		memcpy(mpp.value.data, buf_it, mpp.value.length);
		bool res = pkt_write_c(&mpbuf_end, endof(mpbuf), PV_LEGACY_DEFAULT, NetPacketHeader, {
			.property = PacketProperty_UnconnectedMessage,
			.unconnectedMessage = {
				.type = type,
				.protocolVersion = session->net.version.protocolVersion,
			},
		}) && MASTER_SERIALIZE(&mpp, &mpbuf_end, endof(mpbuf));
		(void)res;
		master_send_internal(net, session, mpbuf, (uint16_t)(mpbuf_end - mpbuf), mpp.value.base.requestId, type != MessageType_HandshakeMessage);
		++partCount;
		buf_it += mpp.value.length;
	} while(buf_it < buf_end);
	return partCount;
}

static void master_send_ack(struct NetContext *net, struct MasterSession *session, MessageType type, uint32_t requestId) {
	uint8_t resp[65536], *resp_end = resp;
	uint8_t ackType;
	switch(type) {
		case MessageType_UserMessage: ackType = UserMessageType_MessageReceivedAcknowledge; break;
		case MessageType_GameLiftMessage: ackType = GameLiftMessageType_MessageReceivedAcknowledge; break;
		case MessageType_HandshakeMessage: ackType = HandshakeMessageType_MessageReceivedAcknowledge; break;
		default: uprintf("Bad MessageType in master_send_ack()\n"); return;
	}
	bool res = pkt_write_c(&resp_end, endof(resp), PV_LEGACY_DEFAULT, NetPacketHeader, {
		.property = PacketProperty_UnconnectedMessage,
		.unconnectedMessage = {
			.type = type,
			.protocolVersion = session->net.version.protocolVersion,
		},
	}) && MASTER_SERIALIZE((&(struct MessageReceivedAcknowledgeProxy){ // TODO: gl
		.type = ackType,
		.value = {
			.base.responseId = requestId,
			.messageHandled = true,
		},
	}), &resp_end, endof(resp));
	if(res)
		net_send_internal(net, &session->net, resp, (uint32_t)(resp_end - resp), type != MessageType_HandshakeMessage);
}

static inline bool InitializeConnection(struct NetContext *net, struct MasterSession *session, const struct ClientHelloRequest *req) {
	if(session->handshake.step != HandshakeMessageType_ClientHelloRequest) {
		// 5 second timeout to prevent clients from getting "locked out" if their previous session hasn't closed or timed out yet
		if(net_time() - NetSession_get_lastKeepAlive(&session->net) < 5000)
			return session->handshake.step == HandshakeMessageType_ClientHelloWithCookieRequest;
		struct SS addr = *NetSession_get_addr(&session->net);
		NetSession_free(&session->net);
		NetSession_init(&session->net, net, addr); // security or something idk
		session->resend.set = COUNTER64_CLEAR;
	}
	session->epoch = req->base.requestId & 0xff000000;
	session->net.clientRandom = req->random;
	session->handshake.step = HandshakeMessageType_ClientHelloWithCookieRequest;
	return true;
}

static void handle_ClientHelloRequest(struct NetContext *net, struct MasterSession *session, const struct ClientHelloRequest *req) {
	if(!InitializeConnection(net, session, req))
		return;
	struct HandshakeMessage r_hello = {
		.type = HandshakeMessageType_HelloVerifyRequest,
		.helloVerifyRequest.base = {
			.requestId = master_getNextRequestId(session),
			.responseId = req->base.requestId,
		},
	};
	r_hello.helloVerifyRequest.cookie = *NetSession_get_cookie(&session->net);
	uint8_t resp[65536], *resp_end = resp;
	if(MASTER_SERIALIZE(&r_hello, &resp_end, endof(resp)))
		master_send(net, session, MessageType_HandshakeMessage, resp, resp_end, false);
}

static void handle_ClientHelloWithCookieRequest(struct MasterContext *ctx, struct NetContext *net, struct MasterSession *session, const struct ClientHelloWithCookieRequest *req) {
	master_send_ack(net, session, MessageType_HandshakeMessage, req->base.requestId);
	if(session->handshake.step != HandshakeMessageType_ClientHelloWithCookieRequest)
		return;
	if(memcmp(req->cookie.raw, NetSession_get_cookie(&session->net)->raw, sizeof(req->cookie.raw)) != 0)
		return;
	if(memcmp(req->random.raw, session->net.clientRandom.raw, sizeof(req->random.raw)) != 0)
		return;

	struct HandshakeMessage r_cert = {
		.type = HandshakeMessageType_ServerCertificateRequest,
		.serverCertificateRequest.base = {
			.requestId = master_getNextRequestId(session),
			.responseId = req->certificateResponseId,
		},
	};
	for(const mbedtls_x509_crt *it = ctx->cert; it != NULL; it = it->next) {
		r_cert.serverCertificateRequest.certificateList[r_cert.serverCertificateRequest.certificateCount].length = (uint32_t)it->raw.len;
		memcpy(r_cert.serverCertificateRequest.certificateList[r_cert.serverCertificateRequest.certificateCount].data, it->raw.p, r_cert.serverCertificateRequest.certificateList[r_cert.serverCertificateRequest.certificateCount].length);
		++r_cert.serverCertificateRequest.certificateCount;
	}

	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_cert, &resp_end, endof(resp)))
		return;
	session->handshake = (struct HandshakeState){
		.certificateRequestId = r_cert.serverCertificateRequest.base.requestId,
		.helloResponseId = req->base.requestId,
		.certificateOutboundCount = master_send(net, session, MessageType_HandshakeMessage, resp, resp_end, true),
		.step = HandshakeMessageType_ServerCertificateRequest,
	};
}

static void handle_ServerCertificateRequest_sent(struct MasterContext *ctx, struct NetContext *net, struct MasterSession *session) {
	if(session->handshake.step != HandshakeMessageType_ServerCertificateRequest)
		return;
	struct HandshakeMessage r_hello = {
		.type = HandshakeMessageType_ServerHelloRequest,
		.serverHelloRequest = {
			.base = {
				.requestId = master_getNextRequestId(session),
				.responseId = session->handshake.helloResponseId,
			},
			.random = *NetKeypair_get_random(&session->net.keys),
		},
	};
	if(NetKeypair_write_key(&session->net.keys, net, &r_hello.serverHelloRequest.publicKey))
		return;
	if(ctx->key != NULL)
		NetSession_signature(&session->net, net, ctx->key, &r_hello.serverHelloRequest.signature);
	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_hello, &resp_end, endof(resp)))
		return;
	master_send(net, session, MessageType_HandshakeMessage, resp, resp_end, true);
	session->handshake.step = HandshakeMessageType_ClientKeyExchangeRequest;
}

static void handle_ClientKeyExchangeRequest(struct NetContext *net, struct MasterSession *session, const struct ClientKeyExchangeRequest *req) {
	master_send_ack(net, session, MessageType_HandshakeMessage, req->base.requestId);
	if(session->handshake.step != HandshakeMessageType_ClientKeyExchangeRequest)
		return;
	if(NetSession_set_remotePublicKey(&session->net, net, &req->clientPublicKey, false))
		return;
	struct HandshakeMessage r_spec = {
		.type = HandshakeMessageType_ChangeCipherSpecRequest,
		.changeCipherSpecRequest.base = {
			.requestId = master_getNextRequestId(session),
			.responseId = req->base.requestId,
		},
	};
	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_spec, &resp_end, endof(resp)))
		return;
	master_send(net, session, MessageType_HandshakeMessage, resp, resp_end, true);
	session->handshake.step = HandshakeMessageType_ChangeCipherSpecRequest;
}

static void handle_BaseAuthenticate(struct NetContext *net, struct MasterSession *session, struct BaseMasterServerReliableResponse req, bool gamelift) {
	_Static_assert((uint32_t)GameLiftMessageType_AuthenticateUserResponse == (uint32_t)UserMessageType_AuthenticateUserResponse, "ABI break");
	master_send_ack(net, session, gamelift ? MessageType_GameLiftMessage : MessageType_UserMessage, req.requestId);
	struct UserMessage r_auth = {
		.type = UserMessageType_AuthenticateUserResponse,
		.authenticateUserResponse = {
			.base = {
				.requestId = master_getNextRequestId(session),
				.responseId = req.requestId,
			},
			.result = AuthenticateUserResponse_Result_Success,
		},
	};
	uint8_t resp[65536], *resp_end = resp;
	if(MASTER_SERIALIZE(&r_auth, &resp_end, endof(resp)))
		master_send(net, session, gamelift ? MessageType_GameLiftMessage : MessageType_UserMessage, resp, resp_end, true);
}

static void handle_AuthenticateUserRequest(struct NetContext *net, struct MasterSession *session, const struct AuthenticateUserRequest *req) {
	handle_BaseAuthenticate(net, session, req->base, false);
}

static void handle_AuthenticateGameLiftUserRequest(struct MasterContext *ctx, struct NetContext *net, struct MasterSession *session, const struct AuthenticateGameLiftUserRequest *req) {
	handle_BaseAuthenticate(net, session, req->base, true);
	ctx->onGraphAuth(net, &session->net, req);
}

/*static struct BitMask128 get_mask(const char *key) {
	uint32_t len = strlen(key);
	uint32_t hash = 0x21 ^ len;
	int32_t num3 = 0;
	while(len >= 4) {
		uint32_t num4 = (key[num3 + 3] << 24) | (key[num3 + 2] << 16) | (key[num3 + 1] << 8) | key[num3];
		num4 *= 1540483477;
		num4 ^= num4 >> 24;
		num4 *= 1540483477;
		hash *= 1540483477;
		hash ^= num4;
		num3 += 4;
		len -= 4;
	}
	switch(len) {
		case 3:
			hash ^= key[num3 + 2] << 16;
		case 2:
			hash ^= key[num3 + 1] << 8;
		case 1:
			hash ^= key[num3];
			hash *= 1540483477;
		case 0:
			break;
	}
	hash ^= hash >> 13;
	hash *= 1540483477;
	hash ^= (hash >> 15);

	struct BitMask128 out = {0, 0};
	for(uint_fast8_t i = 0; i < 2; i++) {
		uint_fast8_t ind = (hash % 128);
		if(ind >= 64)
			out.d0 |= 1LL << (ind - 64);
		else
			out.d1 |= 1LL << ind;
		hash >>= 13;
	}
	return out;
}*/

typedef uint8_t MasterCookieType;
enum {
	MasterCookieType_INVALID,
	MasterCookieType_LocalConnect,
	MasterCookieType_GraphConnect,
};

struct ConnectCookie {
	MasterCookieType cookieType;
	uint32_t room;
};

struct LocalConnectCookie {
	struct ConnectCookie base;
	struct String secret;
	struct SS addr;
	struct BaseMasterServerReliableRequest request;
	struct BeatmapLevelSelectionMask selectionMask;
};

struct GraphConnectCookie {
	struct ConnectCookie base;
	struct String secret;
	const struct WireLink *status;
	WireCookie cookie;
};

static ConnectToServerResponse_Result SendWireSessionAlloc(struct WireSessionAlloc *allocInfo, struct ConnectCookie *state, size_t state_len, struct GameplayServerConfiguration configuration, ServerCode code) {
	state->room = ~UINT32_C(0);
	if(allocInfo->protocolVersion >= 9) {
		uprintf("Connect to Server Error: Game version too new\n");
		return ConnectToServerResponse_Result_VersionMismatch;
	}
	WireCookie cookie = 0;
	bool failed = true;
	struct WireLink *link = NULL;
	if(code == ServerCode_NONE) {
		if(!allocInfo->secret.length) {
			uprintf("Connect to Server Error: Quickplay not supported\n");
			return ConnectToServerResponse_Result_NoAvailableDedicatedServers;
		}
		struct PoolHost *host = pool_handle_new(&state->room, false);
		if(host == NULL) {
			uprintf("Connect to Server Error: pool_handle_new() failed\n");
			return ConnectToServerResponse_Result_NoAvailableDedicatedServers;
		}
		allocInfo->room = state->room;
		link = pool_host_wire(host);
		cookie = WireLink_makeCookie(link, state, state_len);
		failed = WireLink_send(link, &(struct WireMessage){
			.type = WireMessageType_WireRoomSpawn,
			.cookie = cookie,
			.roomSpawn = {
				.base = *allocInfo,
				.configuration = configuration,
			},
		});
	} else {
		struct PoolHost *host = pool_handle_lookup(&state->room, code);
		if(host == NULL) {
			uprintf("Connect to Server Error: Room code does not exist\n");
			return ConnectToServerResponse_Result_InvalidCode;
		}
		allocInfo->room = state->room;
		link = pool_host_wire(host);
		cookie = WireLink_makeCookie(link, state, state_len);
		failed = WireLink_send(link, &(struct WireMessage){
			.type = WireMessageType_WireRoomJoin,
			.cookie = cookie,
			.roomJoin.base = *allocInfo,
		});
	}
	if(failed) {
		WireLink_freeCookie(link, cookie);
		return ConnectToServerResponse_Result_UnknownError;
	}
	return ConnectToServerResponse_Result_Success;
}

static uint32_t shuffle(uint32_t num, bool dir) {
	static const uint32_t magic[2] = {0x45d9f3b, 0x119de1f3};
	num = ((num >> 16) ^ num) * magic[dir];
	num = ((num >> 16) ^ num) * magic[dir];
	num = (num >> 16) ^ num;
	return num;
}

static bool handle_WireSessionAllocResp_local(struct NetContext *net, struct MasterSession *session, struct PoolHost *host, const struct LocalConnectCookie *state, const struct WireSessionAllocResp *sessionAlloc) {
	struct UserMessage r_conn = {
		.type = UserMessageType_ConnectToServerResponse,
		.connectToServerResponse = {
			.base = {
				.requestId = session ? master_getNextRequestId(session) : 0,
				.responseId = state->request.requestId,
			},
			.result = (sessionAlloc != NULL) ? sessionAlloc->result : ConnectToServerResponse_Result_UnknownError,
		},
	};

	if(r_conn.connectToServerResponse.result == ConnectToServerResponse_Result_Success) {
		r_conn.connectToServerResponse = (struct ConnectToServerResponse){
			.base = r_conn.connectToServerResponse.base,
			.result = ConnectToServerResponse_Result_Success,
			.userId = String_fmt("beatupserver:%08x", shuffle((pool_host_ident(host) << 14) | (state->base.room + 1), false)),
			.userName = String_from(""),
			.secret = state->secret,
			.selectionMask = state->selectionMask,
			.isConnectionOwner = true,
			.isDedicatedServer = true,
			.remoteEndPoint = sessionAlloc->endPoint,
			.random = sessionAlloc->random,
			.publicKey = sessionAlloc->publicKey,
			.code = pool_handle_code(host, state->base.room),
			.configuration = sessionAlloc->configuration,
			.managerId = sessionAlloc->managerId,
		};
		uprintf("Sending player to room `%s`\n", ServerCodeToString((char[8]){0}, r_conn.connectToServerResponse.code));
	}

	uint8_t resp[65536], *resp_end = resp;
	if(session && MASTER_SERIALIZE(&r_conn, &resp_end, endof(resp)))
		master_send(net, session, MessageType_UserMessage, resp, resp_end, true);
	return r_conn.connectToServerResponse.result != ConnectToServerResponse_Result_Success;
}

static bool handle_WireSessionAllocResp_graph(struct LocalMasterContext *ctx, struct PoolHost *host, const struct GraphConnectCookie *state, const struct WireSessionAllocResp *sessionAlloc) {
	if(state->status != ctx->status)
		return true;
	struct WireGraphConnectResp resp = {
		.result = MultiplayerPlacementErrorCode_Unknown,
	};
	switch((sessionAlloc != NULL) ? sessionAlloc->result : ConnectToServerResponse_Result_UnknownError) {
		case ConnectToServerResponse_Result_Success: {
			resp = (struct WireGraphConnectResp){
				.result = MultiplayerPlacementErrorCode_Success,
				.configuration = sessionAlloc->configuration,
				.hostId = shuffle((pool_host_ident(host) << 14) | (state->base.room + 1), false),
				.endPoint = sessionAlloc->endPoint,
				.roomSlot = state->base.room,
				.playerSlot = sessionAlloc->playerSlot,
				.code = pool_handle_code(host, state->base.room),
			};
			uprintf("Sending player to room `%s`\n", ServerCodeToString((char[8]){0}, resp.code));
			break;
		}
		case ConnectToServerResponse_Result_InvalidSecret: [[fallthrough]];
		case ConnectToServerResponse_Result_InvalidCode: resp.result = MultiplayerPlacementErrorCode_ServerDoesNotExist; break;
		case ConnectToServerResponse_Result_InvalidPassword: resp.result = MultiplayerPlacementErrorCode_AuthenticationFailed; break;
		case ConnectToServerResponse_Result_ServerAtCapacity: resp.result = MultiplayerPlacementErrorCode_ServerAtCapacity; break;
		case ConnectToServerResponse_Result_NoAvailableDedicatedServers: resp.result = MultiplayerPlacementErrorCode_ServerDoesNotExist; break;
		default:;
	}
	WireLink_send(ctx->status, &(struct WireMessage){
		.cookie = state->cookie,
		.type = WireMessageType_WireGraphConnectResp,
		.graphConnectResp = resp,
	});
	return resp.result != MultiplayerPlacementErrorCode_Success;
}

static void handle_WireGraphConnect(struct LocalMasterContext *ctx, WireCookie cookie, const struct WireGraphConnect *req) {
	struct GraphConnectCookie state = {
		.base.cookieType = MasterCookieType_GraphConnect,
		.secret = req->secret,
		.status = ctx->status,
		.cookie = cookie,
	};
	struct WireSessionAlloc allocInfo = {
		.secret = req->secret,
		.userId = req->userId,
		.ipv4 = true,
		.direct = true,
	};
	const ConnectToServerResponse_Result result = SendWireSessionAlloc(&allocInfo, &state.base, sizeof(state), req->configuration, req->code);
	if(result == ConnectToServerResponse_Result_Success)
		return;
	handle_WireSessionAllocResp_graph(ctx, NULL, &state, &(const struct WireSessionAllocResp){
		.result = result,
	});
}

static void handle_WireSessionAllocResp(struct LocalMasterContext *ctx, struct WireLink *link, struct PoolHost *host, WireCookie cookie, const struct WireSessionAllocResp *sessionAlloc, bool spawn) {
	const struct DataView cookieView = WireLink_getCookie(link, cookie);
	const struct ConnectCookie *state = (struct ConnectCookie*)cookieView.data;
	bool dropped = spawn;
	switch((cookieView.length >= sizeof(*state)) ? state->cookieType : MasterCookieType_INVALID) {
		case MasterCookieType_LocalConnect: {
			if(cookieView.length != sizeof(struct LocalConnectCookie))
				break;
			const struct LocalConnectCookie *const localState = (const struct LocalConnectCookie*)state;
			dropped &= handle_WireSessionAllocResp_local(&ctx->net, MasterContext_lookup(&ctx->base, localState->addr), host, localState, sessionAlloc);
			return;
		}
		case MasterCookieType_GraphConnect: {
			if(cookieView.length != sizeof(struct GraphConnectCookie))
				break;
			dropped &= handle_WireSessionAllocResp_graph(ctx, host, (const struct GraphConnectCookie*)state, sessionAlloc);
			return;
		}
		default: dropped = false;
	}
	if(dropped)
		pool_handle_free(host, state->room);
	uprintf("Connect to Server Error: Malformed wire cookie\n");
}

static void master_onWireMessage_status(struct LocalMasterContext *ctx, struct WireLink *link, const struct WireMessage *message) {
	if(message == NULL) {
		if(link == ctx->status)
			ctx->status = NULL;
		return;
	}
	if(message->type == WireMessageType_WireStatusHook) {
		ctx->status = link;
		return;
	}
	if(link != ctx->status) {
		uprintf("dropping unbound wire message\n");
		return;
	}
	switch(message->type) {
		case WireMessageType_WireGraphConnect: handle_WireGraphConnect(ctx, message->cookie, &message->graphConnect); break;
		default: uprintf("Unhandled wire message [%s]\n", reflect(WireMessageType, message->type));
	}
}

static void master_onWireMessage(struct WireContext *wire, struct WireLink *link, const struct WireMessage *message) {
	struct LocalMasterContext *const ctx = (struct LocalMasterContext*)wire->userptr;
	net_lock(&ctx->net);
	struct PoolHost **const host = (struct PoolHost**)WireLink_userptr(link);
	if(*host == NULL) {
		if(message == NULL || message->type != WireMessageType_WireSetAttribs) {
			master_onWireMessage_status(ctx, link, message);
			goto unlock;
		}
		*host = pool_host_attach(link);
		if(*host == NULL) {
			uprintf("pool_host_attach() failed\n");
			goto unlock;
		}
	}
	if(message == NULL) {
		for(WireCookie cookie = 1; cookie <= WireLink_lastCookieIndex(link); ++cookie) {
			const struct DataView cookieView = WireLink_getCookie(link, cookie);
			const MasterCookieType cookieType = (cookieView.length >= sizeof(cookieType)) ? *(MasterCookieType*)cookieView.data : MasterCookieType_INVALID;
			if(cookieType == MasterCookieType_LocalConnect || cookieType == MasterCookieType_GraphConnect)
				handle_WireSessionAllocResp(ctx, link, *host, cookie, NULL, false); // TODO: retry with a different instance if any are still alive
			WireLink_freeCookie(link, cookie);
		}
		pool_host_detach(*host);
		*host = NULL;
		goto unlock;
	}
	bool spawn = false;
	switch(message->type) {
		case WireMessageType_WireSetAttribs: TEMPpool_host_setAttribs(*host, message->setAttribs.capacity, message->setAttribs.discover); break;
		case WireMessageType_WireRoomSpawnResp: spawn = true; [[fallthrough]];
		case WireMessageType_WireRoomJoinResp: {
			handle_WireSessionAllocResp(ctx, link, *host, message->cookie, &message->roomJoinResp.base, spawn);
			WireLink_freeCookie(link, message->cookie);
			break;
		}
		case WireMessageType_WireRoomCloseNotify: pool_handle_free(*host, message->roomCloseNotify.room); break;
		default: uprintf("Unhandled wire message [%s]\n", reflect(WireMessageType, message->type));
	}
	unlock: net_unlock(&ctx->net);
}

// TODO: deduplicate requests
static void handle_ConnectToServerRequest(struct NetContext *net, struct MasterSession *session, const struct ConnectToServerRequest *req) {
	master_send_ack(net, session, MessageType_UserMessage, req->base.base.requestId);
	struct LocalConnectCookie state = {
		.base.cookieType = MasterCookieType_LocalConnect,
		.secret = req->secret,
		.addr = *NetSession_get_addr(&session->net),
		.request = req->base.base,
		.selectionMask = req->selectionMask,
	};
	/*struct BitMask128 customs = get_mask("custom_levelpack_CustomLevels");
	state.selectionMask.songPacks.bloomFilter.d0 |= customs.d0;
	state.selectionMask.songPacks.bloomFilter.d1 |= customs.d1;*/
	struct WireSessionAlloc allocInfo = {
		.secret = req->secret,
		.userId = req->base.userId,
		.ipv4 = (state.addr.ss.ss_family != AF_INET6 || memcmp(state.addr.in6.sin6_addr.s6_addr, (const uint16_t[]){0,0,0,0,0,0xffff}, 12) == 0),
		.direct = false,
		.random = req->base.random,
		.publicKey = req->base.publicKey,
		.protocolVersion = session->net.version.protocolVersion,
	};
	const ConnectToServerResponse_Result result = SendWireSessionAlloc(&allocInfo, &state.base, sizeof(state), req->configuration, req->code);
	if(result == ConnectToServerResponse_Result_Success)
		return;
	handle_WireSessionAllocResp_local(net, session, NULL, &state, &(const struct WireSessionAllocResp){
		.result = result,
	});
}

static void handle_MultipartMessage(struct MasterContext *ctx, struct NetContext *net, struct MasterSession *session, const struct MultipartMessage *msg) {
	if(!msg->totalLength) {
		uprintf("INVALID MULTIPART LENGTH\n");
		return;
	}
	struct MasterMultipartList **multipart = &session->multipartList;
	for(; *multipart; multipart = &(*multipart)->next) {
		if((*multipart)->id == msg->multipartMessageId) {
			if((*multipart)->totalLength != msg->totalLength) {
				uprintf("BAD MULTIPART LENGTH\n");
				return;
			}
			break;
		}
	}
	if(!*multipart) {
		*multipart = malloc(sizeof(struct MasterMultipartList) + msg->totalLength);
		if(!*multipart) {
			uprintf("alloc error\n");
			abort();
		}
		(*multipart)->next = NULL;
		(*multipart)->id = msg->multipartMessageId;
		(*multipart)->totalLength = msg->totalLength;
		(*multipart)->count = 0;
		memset((*multipart)->data, 0, msg->totalLength);
	}
	if(msg->offset + msg->length > msg->totalLength) {
		uprintf("INVALID MULTIPART LENGTH\n");
		return;
	}
	memcpy(&(*multipart)->data[msg->offset], msg->data, msg->length);
	if(++(*multipart)->count >= (msg->totalLength + sizeof(msg->data) - 1) / sizeof(msg->data)) {
		const uint8_t *data = (*multipart)->data, *end = &(*multipart)->data[msg->totalLength];
		struct UnconnectedMessage header;
		if(pkt_read(&header, &data, end, session->net.version))
			MasterContext_handleMessage(ctx, net, session, header, data, end);
		struct MasterMultipartList *e = *multipart;
		*multipart = (*multipart)->next;
		free(e);
	}
}

static pthread_t master_thread = NET_THREAD_INVALID;
static struct LocalMasterContext localMaster = {
	.net = CLEAR_NETCONTEXT,
}; // TODO: This "singleton" can't scale up due to pool API thread safety
void MasterContext_handleMessage(struct MasterContext *ctx, struct NetContext *net, struct MasterSession *session, struct UnconnectedMessage header, const uint8_t *data, const uint8_t *end) {
	while(data < end) {
		struct SerializeHeader serial;
		if(!pkt_read(&serial, &data, end, session->net.version))
			return;
		const uint8_t *sub = data;
		data += serial.length;
		if(data > end) {
			uprintf("Invalid serial length: %u\n", serial.length);
			return;
		}
		if((uint8_t)header.protocolVersion > session->net.version.protocolVersion)
			session->net.version.protocolVersion = (uint8_t)header.protocolVersion;
		if(header.type == MessageType_UserMessage) {
			session->net.version.direct = false;
			if(ctx != &localMaster.base)
				continue;
			struct UserMessage message = {.type = (UserMessageType)UINT32_C(0xffffffff)};
			pkt_read(&message, &sub, &sub[serial.length], session->net.version);
			if(pkt_debug("BAD USER MESSAGE LENGTH", sub, data, serial.length, session->net.version))
				continue;
			switch(message.type) {
				case UserMessageType_AuthenticateUserRequest: handle_AuthenticateUserRequest(net, session, &message.authenticateUserRequest); break;
				case UserMessageType_AuthenticateUserResponse: uprintf("BAD TYPE: UserMessageType_AuthenticateUserResponse\n"); break;
				case UserMessageType_ConnectToServerResponse: uprintf("BAD TYPE: UserMessageType_ConnectToServerResponse\n"); break;
				case UserMessageType_ConnectToServerRequest: handle_ConnectToServerRequest(net, session, &message.connectToServerRequest); break;
				case UserMessageType_MessageReceivedAcknowledge: master_handle_ack(session, &message.messageReceivedAcknowledge); break;
				case UserMessageType_MultipartMessage: handle_MultipartMessage(ctx, net, session, &message.multipartMessage); break;
				case UserMessageType_SessionKeepaliveMessage: break;
				case UserMessageType_GetPublicServersRequest: uprintf("UserMessageType_GetPublicServersRequest not implemented\n"); abort();
				case UserMessageType_GetPublicServersResponse: uprintf("BAD TYPE: UserMessageType_GetPublicServersResponse\n"); break;
				default: uprintf("BAD USER MESSAGE TYPE: %hhu\n", message.type);
			}
			continue;
		}
		if(header.type == MessageType_GameLiftMessage) {
			session->net.version.direct = true;
			if(ctx == &localMaster.base)
				continue;
			struct GameLiftMessage message = {.type = (GameLiftMessageType)UINT32_C(0xffffffff)};
			pkt_read(&message, &sub, &sub[serial.length], session->net.version);
			if(pkt_debug("BAD GAMELIFT MESSAGE LENGTH", sub, data, serial.length, session->net.version))
				continue;
			switch(message.type) {
				case GameLiftMessageType_AuthenticateGameLiftUserRequest: handle_AuthenticateGameLiftUserRequest(ctx, net, session, &message.authenticateGameLiftUserRequest); break;
				case GameLiftMessageType_AuthenticateUserResponse: uprintf("BAD TYPE: GameLiftMessageType_AuthenticateUserResponse\n"); break;
				case GameLiftMessageType_MessageReceivedAcknowledge: master_handle_ack(session, &message.messageReceivedAcknowledge); break;
				case GameLiftMessageType_MultipartMessage: handle_MultipartMessage(ctx, net, session, &message.multipartMessage); break;
				default: uprintf("BAD GAMELIFT MESSAGE TYPE: %hhu\n", message.type);
			}
			continue;
		}
		if(header.type == MessageType_HandshakeMessage) {
			struct HandshakeMessage message = {.type = (HandshakeMessageType)UINT32_C(0xffffffff)};
			pkt_read(&message, &sub, &sub[serial.length], session->net.version);
			if(pkt_debug("BAD HANDSHAKE MESSAGE LENGTH", sub, data, serial.length, session->net.version))
				continue;
			switch(message.type) {
				case HandshakeMessageType_ClientHelloRequest: handle_ClientHelloRequest(net, session, &message.clientHelloRequest); break;
				case HandshakeMessageType_HelloVerifyRequest: uprintf("BAD TYPE: HandshakeMessageType_HelloVerifyRequest\n"); break;
				case HandshakeMessageType_ClientHelloWithCookieRequest: handle_ClientHelloWithCookieRequest(ctx, net, session, &message.clientHelloWithCookieRequest); break;
				case HandshakeMessageType_ServerHelloRequest: uprintf("BAD TYPE: HandshakeMessageType_ServerHelloRequest\n"); break;
				case HandshakeMessageType_ServerCertificateRequest: uprintf("BAD TYPE: HandshakeMessageType_ServerCertificateRequest\n"); break;
				case HandshakeMessageType_ClientKeyExchangeRequest: handle_ClientKeyExchangeRequest(net, session, &message.clientKeyExchangeRequest); break;
				case HandshakeMessageType_ChangeCipherSpecRequest: uprintf("BAD TYPE: HandshakeMessageType_ChangeCipherSpecRequest\n"); break;
				case HandshakeMessageType_MessageReceivedAcknowledge: {
					const struct MasterPacket *sent = master_handle_ack(session, &message.messageReceivedAcknowledge);
					if(sent == NULL)
						break;
					struct MultipartMessageReadbackProxy readback;
					if(!pkt_read(&readback, (const uint8_t*[]){sent->data}, &sent->data[sent->length], PV_LEGACY_DEFAULT))
						break;
					if(readback.header.unconnectedMessage.type != MessageType_HandshakeMessage ||
					   readback.type != HandshakeMessageType_MultipartMessage ||
					   readback.multipartMessageId != session->handshake.certificateRequestId)
						break;
					if(--session->handshake.certificateOutboundCount == 0)
						handle_ServerCertificateRequest_sent(ctx, net, session);
					break;
				}
				case HandshakeMessageType_MultipartMessage: uprintf("BAD TYPE: HandshakeMessageType_HandshakeMultipartMessage\n"); break;
				default: uprintf("BAD HANDSHAKE MESSAGE TYPE: %hhu\n", message.type);
			}
			continue;
		}
		uprintf("BAD MESSAGE TYPE: %u\n", header.type);
	}
}

void MasterContext_handle(struct MasterContext *ctx, struct NetContext *net, struct MasterSession *session, const uint8_t *data, const uint8_t *end) {
	struct NetPacketHeader header;
	if(!pkt_read(&header, &data, end, session->net.version))
		return;
	if(header.property == PacketProperty_UnconnectedMessage)
		MasterContext_handleMessage(ctx, net, session, header.unconnectedMessage, data, end);
	else if(header.property != PacketProperty_Disconnect)
		uprintf("Unhandled property [%s]\n", reflect(PacketProperty, header.property));
}

static void *master_handler(struct LocalMasterContext *ctx) {
	net_lock(&ctx->net);
	uprintf("Started\n");
	uint8_t pkt[1536];
	memset(pkt, 0, sizeof(pkt));
	uint32_t len;
	struct MasterSession *session;
	while((len = net_recv(&ctx->net, pkt, (struct NetSession**)&session, NULL)))
		MasterContext_handle(&ctx->base, &ctx->net, session, pkt, &pkt[len]);
	net_unlock(&ctx->net);
	return 0;
}

static void onGraphAuth_stub(struct NetContext*, struct NetSession*, const struct AuthenticateGameLiftUserRequest*) {}
void MasterContext_init(struct MasterContext *ctx) {
	*ctx = (struct MasterContext){
		.onGraphAuth = onGraphAuth_stub,
	};
}

bool MasterContext_setCertificate(struct MasterContext *ctx, const mbedtls_x509_crt *cert, const mbedtls_pk_context *key) {
	uint_fast8_t count = 0;
	for(const mbedtls_x509_crt *it = cert; it != NULL; it = it->next, ++count) {
		if(it->raw.len > 4096) {
			uprintf("Host certificate too large\n");
			return true;
		}
	}
	if(count > lengthof(((struct ServerCertificateRequest*)NULL)->certificateList)) {
		uprintf("Host certificate chain too long\n");
		return true;
	}
	ctx->cert = cert;
	ctx->key = key;
	return false;
}

void MasterContext_cleanup(struct MasterContext *ctx) {
	while(ctx->sessionList)
		ctx->sessionList = master_disconnect(ctx->sessionList);
}

struct WireContext *master_init(const mbedtls_x509_crt *cert, const mbedtls_pk_context *key, uint16_t port) {
	if(net_init(&localMaster.net, port)) {
		uprintf("net_init() failed\n");
		return NULL;
	}
	if(WireContext_init(&localMaster.wire, &localMaster, 16)) {
		uprintf("WireContext_init() failed\n");
		goto fail0;
	}
	MasterContext_init(&localMaster.base);
	if(MasterContext_setCertificate(&localMaster.base, cert, key))
		goto fail1;
	localMaster.net.userptr = &localMaster;
	localMaster.net.onResolve = master_onResolve;
	localMaster.net.onResend = master_onResend;
	localMaster.wire.onMessage = master_onWireMessage;
	if(pthread_create(&master_thread, NULL, (void *(*)(void*))master_handler, &localMaster)) {
		master_thread = NET_THREAD_INVALID;
		goto fail2;
	}
	return &localMaster.wire;
	fail2: MasterContext_cleanup(&localMaster.base);
	fail1: WireContext_cleanup(&localMaster.wire);
	fail0: net_cleanup(&localMaster.net);
	return NULL;
}

void master_cleanup() {
	if(master_thread != NET_THREAD_INVALID) {
		net_stop(&localMaster.net);
		uprintf("Stopping\n");
		pthread_join(master_thread, NULL);
		master_thread = NET_THREAD_INVALID;
		MasterContext_cleanup(&localMaster.base);
	}
	pool_reset();
	WireContext_cleanup(&localMaster.wire);
	net_cleanup(&localMaster.net);
}
