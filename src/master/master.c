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
	uint32_t timeStamp;
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

struct Context {
	struct NetContext net;
	const mbedtls_x509_crt *cert;
	const mbedtls_pk_context *key;
	struct MasterSession *sessionList;
};

static struct MasterSession *master_lookup_session(struct Context *ctx, struct SS addr) {
	for(struct MasterSession *session = ctx->sessionList; session; session = session->next)
		if(SS_equal(&addr, NetSession_get_addr(&session->net)))
			return session;
	return NULL;
}

static struct NetSession *master_onResolve(struct Context *ctx, struct SS addr, void**) {
	struct MasterSession *session = master_lookup_session(ctx, addr);
	if(session)
			return &session->net;
	session = malloc(sizeof(struct MasterSession));
	if(!session) {
		uprintf("alloc error\n");
		abort();
	}
	net_session_init(&ctx->net, &session->net, addr);
	session->lastSentRequestId = 0;
	session->handshake.step = HandshakeMessageType_ClientHelloRequest;
	session->resend.set = COUNTER64_CLEAR;
	session->multipartList = NULL;
	session->next = ctx->sessionList;
	ctx->sessionList = session;

	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(&addr, addrstr);
	uprintf("connect %s\n", addrstr);
	return &session->net;
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
	net_session_free(&session->net);
	free(session);
	return next;
}

static uint32_t master_onResend(struct Context *ctx, uint32_t currentTime) {
	int32_t nextTick = 180000;
	for(struct MasterSession **sp = &ctx->sessionList; *sp;) {
		int32_t kickTime = (int32_t)(NetSession_get_lastKeepAlive(&(*sp)->net) + 180000 - currentTime);
		if(kickTime < 0) { // this filters the RFC-1149 user
			*sp = master_disconnect(*sp);
			continue;
		}
		if(kickTime < nextTick)
			nextTick = kickTime;
		struct MasterSession *session = *sp;
		struct Counter64 iter = session->resend.set;
		for(uint32_t i; Counter64_clear_next(&iter, &i);) {
			int32_t nextSend = (int32_t)(session->resend.slots[i].timeStamp + NET_RESEND_DELAY - currentTime);
			if(session->resend.slots[i].length && nextSend < 0) {
				net_send_internal(&ctx->net, &session->net, session->resend.slots[i].data, session->resend.slots[i].length, session->resend.slots[i].encrypt);
				for(; nextSend < 0; nextSend += NET_RESEND_DELAY)
					session->resend.slots[i].timeStamp += NET_RESEND_DELAY;
			}
			if(nextSend < nextTick)
				nextTick = nextSend;
		}
		sp = &(*sp)->next;
	}
	return (uint32_t)nextTick;
}

static uint32_t master_getNextRequestId(struct MasterSession *session) {
	++session->lastSentRequestId;
	return (session->lastSentRequestId & 63) | session->epoch;
}

// return is valid until the next call to `master_send()`
static const struct MasterPacket *master_handle_ack(struct MasterSession *session, const struct MessageReceivedAcknowledge *ack) {
	uint32_t requestId = ack->base.responseId;
	struct Counter64 iter = session->resend.set;
	for(uint32_t i; Counter64_clear_next(&iter, &i);) {
		if(requestId != session->resend.requestIds[i])
			continue;
		Counter64_clear(&session->resend.set, i);
		return &session->resend.slots[i];
	}
	return NULL;
}

static void master_send_internal(struct NetContext *ctx, struct MasterSession *session, const uint8_t *buf, uint16_t length, uint32_t requestId, bool encrypt) {
	uint32_t i;
	if(Counter64_set_next(&session->resend.set, &i)) {
		struct MasterPacket *slot = &session->resend.slots[i];
		slot->timeStamp = net_time();
		slot->length = length;
		slot->encrypt = encrypt;
		memcpy(slot->data, buf, length);
		session->resend.requestIds[i] = requestId;
	} else {
		uprintf("RESEND BUFFER FULL\n");
	}
	net_send_internal(ctx, &session->net, buf, length, encrypt);
}

static uint32_t read_requestId(const uint8_t *data, const uint8_t *data_end, struct PacketContext version) {
	struct MasterServerReliableRequestProxy out = {0};
	pkt_read(&(struct SerializeHeader){0}, &data, data_end, version);
	pkt_read(&out, &data, data_end, version);
	return out.value.requestId;
}

static uint32_t master_send(struct NetContext *ctx, struct MasterSession *session, MessageType type, const uint8_t *data, const uint8_t *data_end) {
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
		if(res)
			master_send_internal(ctx, session, buf, (uint16_t)(buf_end - buf), read_requestId(data, data_end, PV_LEGACY_DEFAULT), type != MessageType_HandshakeMessage);
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
		case MessageType_HandshakeMessage: mpp.type = HandshakeMessageType_MultipartMessage; break;
		default: uprintf("Bad MessageType in master_send()\b"); return 0;
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
		master_send_internal(ctx, session, mpbuf, (uint16_t)(mpbuf_end - mpbuf), mpp.value.base.requestId, type != MessageType_HandshakeMessage);
		++partCount;
		buf_it += mpp.value.length;
	} while(buf_it < buf_end);
	return partCount;
}

static void master_send_ack(struct Context *ctx, struct MasterSession *session, MessageType type, uint32_t requestId) {
	uint8_t resp[65536], *resp_end = resp;
	bool res = pkt_write_c(&resp_end, endof(resp), PV_LEGACY_DEFAULT, NetPacketHeader, {
		.property = PacketProperty_UnconnectedMessage,
		.unconnectedMessage = {
			.type = type,
			.protocolVersion = session->net.version.protocolVersion,
		},
	}) && MASTER_SERIALIZE((&(struct MessageReceivedAcknowledgeProxy){
		.type = (type == MessageType_UserMessage) ? UserMessageType_MessageReceivedAcknowledge : HandshakeMessageType_MessageReceivedAcknowledge,
		.value = {
			.base.responseId = requestId,
			.messageHandled = true,
		},
	}), &resp_end, endof(resp));
	if(res)
		net_send_internal(&ctx->net, &session->net, resp, (uint32_t)(resp_end - resp), type != MessageType_HandshakeMessage);
}

static void handle_ClientHelloRequest(struct Context *ctx, struct MasterSession *session, const struct ClientHelloRequest *req) {
	if(session->handshake.step != HandshakeMessageType_ClientHelloRequest) {
		if(net_time() - NetSession_get_lastKeepAlive(&session->net) < 5000) // 5 second timeout to prevent clients from getting "locked out" if their previous session hasn't closed or timed out yet
			return;
		struct SS addr = *NetSession_get_addr(&session->net);
		net_session_free(&session->net);
		net_session_init(&ctx->net, &session->net, addr); // security or something idk
		session->resend.set = COUNTER64_CLEAR;
	}
	session->epoch = req->base.requestId & 0xff000000;
	session->net.clientRandom = req->random;
	struct HandshakeMessage r_hello = {
		.type = HandshakeMessageType_HelloVerifyRequest,
		.helloVerifyRequest.base.responseId = req->base.requestId,
	};
	r_hello.helloVerifyRequest.cookie = *NetSession_get_cookie(&session->net);
	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_hello, &resp_end, endof(resp)))
		return;
	master_send(&ctx->net, session, MessageType_HandshakeMessage, resp, resp_end);
	session->handshake.step = HandshakeMessageType_ClientHelloWithCookieRequest;
}

static void handle_ClientHelloWithCookieRequest(struct Context *ctx, struct MasterSession *session, const struct ClientHelloWithCookieRequest *req) {
	master_send_ack(ctx, session, MessageType_HandshakeMessage, req->base.requestId);
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
	for(const mbedtls_x509_crt *it = ctx->cert; it; it = it->next) {
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
		.certificateOutboundCount = master_send(&ctx->net, session, MessageType_HandshakeMessage, resp, resp_end),
		.step = HandshakeMessageType_ServerCertificateRequest,
	};
}

static void handle_ServerCertificateRequest_sent(struct Context *ctx, struct MasterSession *session) {
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
	if(NetKeypair_write_key(&session->net.keys, &ctx->net, &r_hello.serverHelloRequest.publicKey))
		return;
	NetSession_signature(&session->net, &ctx->net, ctx->key, &r_hello.serverHelloRequest.signature);

	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_hello, &resp_end, endof(resp)))
		return;
	master_send(&ctx->net, session, MessageType_HandshakeMessage, resp, resp_end);
	session->handshake.step = HandshakeMessageType_ClientKeyExchangeRequest;
}

static void handle_ClientKeyExchangeRequest(struct Context *ctx, struct MasterSession *session, const struct ClientKeyExchangeRequest *req) {
	master_send_ack(ctx, session, MessageType_HandshakeMessage, req->base.requestId);
	if(session->handshake.step != HandshakeMessageType_ClientKeyExchangeRequest)
		return;
	if(NetSession_set_remotePublicKey(&session->net, &ctx->net, &req->clientPublicKey, false))
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
	master_send(&ctx->net, session, MessageType_HandshakeMessage, resp, resp_end);
	session->handshake.step = HandshakeMessageType_ChangeCipherSpecRequest;
}

static void handle_AuthenticateUserRequest(struct Context *ctx, struct MasterSession *session, const struct AuthenticateUserRequest *req) {
	master_send_ack(ctx, session, MessageType_UserMessage, req->base.requestId);
	struct UserMessage r_auth = {
		.type = UserMessageType_AuthenticateUserResponse,
		.authenticateUserResponse = {
			.base = {
				.requestId = master_getNextRequestId(session),
				.responseId = req->base.requestId,
			},
			.result = AuthenticateUserResponse_Result_Success,
		},
	};
	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_auth, &resp_end, endof(resp)))
		return;
	master_send(&ctx->net, session, MessageType_UserMessage, resp, resp_end);
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
	MasterCookieType_ConnectToServer,
};

struct ConnectToServerCookie {
	MasterCookieType cookieType;
	struct SS addr;
	struct BaseMasterServerReliableRequest request;
	uint32_t room;
	struct String secret;
	struct BeatmapLevelSelectionMask selectionMask;
};

static void handle_WireSessionAllocResp(struct Context *ctx, struct PoolHost *host, uint32_t cookie, const struct WireSessionAllocResp *sessionAlloc, bool spawn) {
	const struct ConnectToServerCookie *state = wire_getCookie(&ctx->net, cookie);
	if(state == NULL || state->cookieType != MasterCookieType_ConnectToServer) {
		uprintf("Connect to Server Error: Malformed wire cookie\n");
		return;
	}

	struct MasterSession *session = master_lookup_session(ctx, state->addr);
	struct UserMessage r_conn = {
		.type = UserMessageType_ConnectToServerResponse,
		.connectToServerResponse = {
			.base = {
				.requestId = session ? master_getNextRequestId(session) : 0,
				.responseId = state->request.requestId,
			},
			.result = sessionAlloc ? sessionAlloc->result : ConnectToServerResponse_Result_UnknownError,
		},
	};

	if(r_conn.connectToServerResponse.result == ConnectToServerResponse_Result_Success) {
		r_conn.connectToServerResponse = (struct ConnectToServerResponse){
			.base = r_conn.connectToServerResponse.base,
			.result = ConnectToServerResponse_Result_Success,
			.userId = String_fmt("beatupserver:%08x", rand()), // TODO: use meaningful id here
			.userName = String_from(""),
			.secret = state->secret,
			.selectionMask = state->selectionMask,
			.isConnectionOwner = true,
			.isDedicatedServer = true,
			.remoteEndPoint = sessionAlloc->endPoint,
			.random = sessionAlloc->random,
			.publicKey = sessionAlloc->publicKey,
			.code = pool_handle_code(host, state->room),
			.configuration = sessionAlloc->configuration,
			.managerId = sessionAlloc->managerId,
		};
		char scode[8];
		uprintf("Sending player to room `%s`\n", ServerCodeToString(scode, r_conn.connectToServerResponse.code));
	} else if(spawn) {
		pool_handle_free(host, state->room);
	}

	uint8_t resp[65536], *resp_end = resp;
	if(session && MASTER_SERIALIZE(&r_conn, &resp_end, endof(resp)))
		master_send(&ctx->net, session, MessageType_UserMessage, resp, resp_end);
}

static void master_onWireMessage(struct Context *ctx, union WireLink *link, const struct WireMessage *message) {
	struct PoolHost *host = pool_host_lookup(link);
	if(!host) {
		uprintf("pool_host_lookup() failed\n"); // If we hit this, something went horribly wrong
		if(message) {
			wire_disconnect(&ctx->net, link);
		} else {
			for(uint32_t cookie = 0; (cookie = wire_nextCookie(&ctx->net, cookie));)
				wire_releaseCookie(&ctx->net, cookie);
		}
		return;
	}
	if(!message) {
		for(uint32_t cookie = 0; (cookie = wire_nextCookie(&ctx->net, cookie));) {
			if(cookie == MasterCookieType_ConnectToServer) // TODO: retry with a different instance if any are still alive
				handle_WireSessionAllocResp(ctx, host, cookie, NULL, false);
			wire_releaseCookie(&ctx->net, cookie);
		}
		pool_host_detach(host);
		return;
	}
	switch(message->type) {
		case WireMessageType_WireSetAttribs: TEMPpool_host_setAttribs(host, message->setAttribs.capacity, message->setAttribs.discover); break;
		case WireMessageType_WireRoomSpawnResp: handle_WireSessionAllocResp(ctx, host, message->cookie, &message->roomSpawnResp.base, true); break;
		case WireMessageType_WireRoomJoinResp: handle_WireSessionAllocResp(ctx, host, message->cookie, &message->roomJoinResp.base, false); break;
		case WireMessageType_WireRoomCloseNotify: pool_handle_free(host, message->roomCloseNotify.room); break;
		default: uprintf("UNHANDLED WIRE MESSAGE [%s]\n", reflect(WireMessageType, message->type));
	}
	wire_releaseCookie(&ctx->net, message->cookie);
}

// TODO: more consistent naming
static void SendConnectError(struct Context *ctx, struct MasterSession *session, struct BaseMasterServerReliableRequest request, ConnectToServerResponse_Result result) {
	struct UserMessage r_conn = {
		.type = UserMessageType_ConnectToServerResponse,
		.connectToServerResponse = {
			.base = {
				.requestId = master_getNextRequestId(session),
				.responseId = request.requestId,
			},
			.result = result,
		},
	};

	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_conn, &resp_end, endof(resp)))
		return;
	master_send(&ctx->net, session, MessageType_UserMessage, resp, resp_end);
}

static void handle_ConnectToServerRequest(struct Context *ctx, struct MasterSession *session, const struct ConnectToServerRequest *req) {
	master_send_ack(ctx, session, MessageType_UserMessage, req->base.base.requestId);
	// TODO: do we need to deduplicate this request?
	struct ConnectToServerCookie state = {
		.cookieType = MasterCookieType_ConnectToServer,
		.addr = *NetSession_get_addr(&session->net),
		.request = req->base.base,
		.room = ~0u,
		.secret = req->secret,
		.selectionMask = req->selectionMask,
	};
	if(session->net.version.protocolVersion >= 9) {
		uprintf("Connect to Server Error: Game version too new\n");
		SendConnectError(ctx, session, state.request, ConnectToServerResponse_Result_VersionMismatch);
		return;
	}
	struct WireSessionAlloc sessionAllocData = {
		.address.length = state.addr.len,
		.secret = req->secret,
		.userId = req->base.userId,
		.userName = req->base.userName,
		.random = req->base.random,
		.publicKey = req->base.publicKey,
		.protocolVersion = session->net.version.protocolVersion,
	};
	memcpy(sessionAllocData.address.data, &state.addr.ss, state.addr.len);
	uint32_t cookie;
	bool failed;
	if(req->code == ServerCode_NONE) {
		if(!req->secret.length) {
			uprintf("Connect to Server Error: Quickplay not supported\n");
			SendConnectError(ctx, session, state.request, ConnectToServerResponse_Result_NoAvailableDedicatedServers); // Quick Play not yet available
			return;
		}
		struct PoolHost *host = pool_handle_new(&state.room, false);
		if(!host) {
			uprintf("Connect to Server Error: pool_handle_new() failed\n");
			SendConnectError(ctx, session, state.request, ConnectToServerResponse_Result_NoAvailableDedicatedServers);
			return;
		}
		sessionAllocData.room = state.room;
		cookie = wire_reserveCookie(&ctx->net, &state, sizeof(state));
		failed = wire_send(&ctx->net, pool_host_wire(host), &(struct WireMessage){
			.type = WireMessageType_WireRoomSpawn,
			.cookie = cookie,
			.roomSpawn = {
				.base = sessionAllocData,
				.configuration = req->configuration,
			},
		});
	} else {
		struct PoolHost *host = pool_handle_lookup(&state.room, req->code);
		if(!host) {
			uprintf("Connect to Server Error: Room code does not exist\n");
			SendConnectError(ctx, session, state.request, ConnectToServerResponse_Result_InvalidCode);
			return;
		}
		sessionAllocData.room = state.room;
		cookie = wire_reserveCookie(&ctx->net, &state, sizeof(state));
		failed = wire_send(&ctx->net, pool_host_wire(host), &(struct WireMessage){
			.type = WireMessageType_WireRoomJoin,
			.cookie = cookie,
			.roomJoin.base = sessionAllocData,
		});
	}
	if(failed) {
		wire_releaseCookie(&ctx->net, cookie);
		SendConnectError(ctx, session, state.request, ConnectToServerResponse_Result_UnknownError);
	}
	/*struct BitMask128 customs = get_mask("custom_levelpack_CustomLevels");
	req->selectionMask.songPacks.bloomFilter.d0 |= customs.d0;
	req->selectionMask.songPacks.bloomFilter.d1 |= customs.d1;*/
}

static void handle_packet(struct Context *ctx, struct MasterSession *session, struct UnconnectedMessage message, const uint8_t *data, const uint8_t *end);
static void handle_MultipartMessage(struct Context *ctx, struct MasterSession *session, const struct MultipartMessage *msg) {
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
			handle_packet(ctx, session, header, data, end);
		struct MasterMultipartList *e = *multipart;
		*multipart = (*multipart)->next;
		free(e);
	}
}

static void handle_packet(struct Context *ctx, struct MasterSession *session, struct UnconnectedMessage header, const uint8_t *data, const uint8_t *end) {
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
		if(header.type == MessageType_UserMessage) {
			if(header.protocolVersion > session->net.version.protocolVersion)
				session->net.version.protocolVersion = (uint8_t)header.protocolVersion;
			struct UserMessage message = {.type = (UserMessageType)UINT32_C(0xffffffff)};
			pkt_read(&message, &sub, &sub[serial.length], session->net.version);
			if(pkt_debug("BAD USER MESSAGE LENGTH", sub, data, serial.length, session->net.version))
				continue;
			switch(message.type) {
				case UserMessageType_AuthenticateUserRequest: handle_AuthenticateUserRequest(ctx, session, &message.authenticateUserRequest); break;
				case UserMessageType_AuthenticateUserResponse: uprintf("BAD TYPE: UserMessageType_AuthenticateUserResponse\n"); break;
				case UserMessageType_ConnectToServerResponse: uprintf("BAD TYPE: UserMessageType_ConnectToServerResponse\n"); break;
				case UserMessageType_ConnectToServerRequest: handle_ConnectToServerRequest(ctx, session, &message.connectToServerRequest); break;
				case UserMessageType_MessageReceivedAcknowledge: master_handle_ack(session, &message.messageReceivedAcknowledge); break;
				case UserMessageType_MultipartMessage: handle_MultipartMessage(ctx, session, &message.multipartMessage); break;
				case UserMessageType_SessionKeepaliveMessage: break;
				case UserMessageType_GetPublicServersRequest: uprintf("UserMessageType_GetPublicServersRequest not implemented\n"); abort();
				case UserMessageType_GetPublicServersResponse: uprintf("BAD TYPE: UserMessageType_GetPublicServersResponse\n"); break;
				default: uprintf("BAD USER MESSAGE TYPE: %hhu\n", message.type);
			}
		} else if(header.type == MessageType_HandshakeMessage) {
			if(header.protocolVersion > session->net.version.protocolVersion)
				session->net.version.protocolVersion = (uint8_t)header.protocolVersion;
			struct HandshakeMessage message = {.type = (HandshakeMessageType)UINT32_C(0xffffffff)};
			pkt_read(&message, &sub, &sub[serial.length], session->net.version);
			if(pkt_debug("BAD HANDSHAKE MESSAGE LENGTH", sub, data, serial.length, session->net.version))
				continue;
			switch(message.type) {
				case HandshakeMessageType_ClientHelloRequest: handle_ClientHelloRequest(ctx, session, &message.clientHelloRequest); break;
				case HandshakeMessageType_HelloVerifyRequest: uprintf("BAD TYPE: HandshakeMessageType_HelloVerifyRequest\n"); break;
				case HandshakeMessageType_ClientHelloWithCookieRequest: handle_ClientHelloWithCookieRequest(ctx, session, &message.clientHelloWithCookieRequest); break;
				case HandshakeMessageType_ServerHelloRequest: uprintf("BAD TYPE: HandshakeMessageType_ServerHelloRequest\n"); break;
				case HandshakeMessageType_ServerCertificateRequest: uprintf("BAD TYPE: HandshakeMessageType_ServerCertificateRequest\n"); break;
				case HandshakeMessageType_ClientKeyExchangeRequest: handle_ClientKeyExchangeRequest(ctx, session, &message.clientKeyExchangeRequest); break;
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
						handle_ServerCertificateRequest_sent(ctx, session);
					break;
				}
				case HandshakeMessageType_MultipartMessage: uprintf("BAD TYPE: HandshakeMessageType_HandshakeMultipartMessage\n"); break;
				default: uprintf("BAD HANDSHAKE MESSAGE TYPE: %hhu\n", message.type);
			}
		} else {
			uprintf("BAD MESSAGE TYPE: %u\n", header.type);
		}
	}
}

static void *master_handler(struct Context *ctx) {
	net_lock(&ctx->net);
	uprintf("Started\n");
	uint8_t pkt[1536];
	memset(pkt, 0, sizeof(pkt));
	uint32_t len;
	struct MasterSession *session;
	while((len = net_recv(&ctx->net, pkt, (struct NetSession**)&session, NULL))) {
		const uint8_t *data = pkt, *end = &pkt[len];
		struct NetPacketHeader header;
		if(!pkt_read(&header, &data, end, PV_LEGACY_DEFAULT))
			continue;
		if(header.property == PacketProperty_UnconnectedMessage)
			handle_packet(ctx, session, header.unconnectedMessage, data, end);
		else
			uprintf("Unsupported packet type: %s\n", reflect(PacketProperty, header.property));
	}
	net_unlock(&ctx->net);
	return 0;
}

static void master_onWireLink(struct Context*, union WireLink *link) {
	struct PoolHost *host = pool_host_attach((union WireLink*)link);
	if(!host)
		uprintf("TEMPwire_attach_local() failed\n");
}

static pthread_t master_thread = NET_THREAD_INVALID;
static struct Context ctx = {CLEAR_NETCONTEXT, NULL, NULL, NULL}; // TODO: This "singleton" can't actually scale up due to the pool API no longer being threadsafe
struct NetContext *master_init(const mbedtls_x509_crt *cert, const mbedtls_pk_context *key, uint16_t port) {
	if(net_init(&ctx.net, port, false, 16)) {
		uprintf("net_init() failed\n");
		return NULL;
	}
	uint_fast8_t count = 0;
	for(const mbedtls_x509_crt *it = cert; it; it = it->next, ++count) {
		if(it->raw.len > 4096) {
			uprintf("Host certificate too large\n");
			return NULL;
		}
	}
	if(count > lengthof(((struct ServerCertificateRequest*)NULL)->certificateList)) {
		uprintf("Host certificate chain too long\n");
		return NULL;
	}
	ctx.cert = cert;
	ctx.key = key;
	ctx.net.userptr = &ctx;
	ctx.net.onResolve = (struct NetSession *(*)(void*, struct SS, void**))master_onResolve;
	ctx.net.onResend = (uint32_t (*)(void*, uint32_t))master_onResend;
	ctx.net.onWireLink = (void (*)(void*, union WireLink*))master_onWireLink;
	ctx.net.onWireMessage = (void (*)(void*, union WireLink*, const struct WireMessage*))master_onWireMessage;
	if(pthread_create(&master_thread, NULL, (void *(*)(void*))master_handler, &ctx)) {
		master_thread = NET_THREAD_INVALID;
		return NULL;
	}
	return &ctx.net;
}

void master_cleanup() {
	if(master_thread != NET_THREAD_INVALID) {
		net_stop(&ctx.net);
		uprintf("Stopping\n");
		pthread_join(master_thread, NULL);
		master_thread = NET_THREAD_INVALID;
		while(ctx.sessionList)
			ctx.sessionList = master_disconnect(ctx.sessionList);
	}
	pool_reset(&ctx.net);
	net_cleanup(&ctx.net);
}
