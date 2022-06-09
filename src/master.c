#include "global.h"
#include "instance/instance.h"
#include "thread.h"
#include "pool.h"
#include "scramble.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MASTER_WINDOW_SIZE 64
#define MASTER_SERIALIZE(data, pkt, end) pkt_serialize(data, pkt, end, PV_LEGACY_DEFAULT)

struct MasterPacket {
	uint32_t len;
	MessageType messageType;
	uint8_t serialType;
	uint8_t data[512];
};
struct MasterResendSparsePtr {
	bool shouldSend;
	bool encrypt;
	uint32_t timeStamp;
	uint32_t requestId;
	uint32_t data;
};
struct MasterResend {
	uint32_t count;
	struct MasterResendSparsePtr index[MASTER_WINDOW_SIZE];
	struct MasterPacket data[MASTER_WINDOW_SIZE];
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
	uint32_t ClientHelloWithCookieRequest_requestId;
	HandshakeMessageType handshakeStep;
	struct MasterResend resend;
	struct MasterMultipartList *multipartList;
};

struct Context {
	struct NetContext net;
	const mbedtls_x509_crt *cert;
	const mbedtls_pk_context *key;
	struct MasterSession *sessionList;
};

static struct NetSession *master_onResolve(struct Context *ctx, struct SS addr, void **userdata_out) {
	struct MasterSession *session = ctx->sessionList;
	for(; session; session = session->next)
		if(addrs_are_equal(&addr, NetSession_get_addr(&session->net)))
			return &session->net;
	session = malloc(sizeof(struct MasterSession));
	if(!session) {
		uprintf("alloc error\n");
		abort();
	}
	net_session_init(&ctx->net, &session->net, addr);
	session->lastSentRequestId = 0;
	session->handshakeStep = 255;
	session->resend.count = 0;
	for(uint32_t i = 0; i < MASTER_WINDOW_SIZE; ++i)
		session->resend.index[i].data = i;
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

static void master_onResend(struct Context *ctx, uint32_t currentTime, uint32_t *nextTick) {
	for(struct MasterSession **sp = &ctx->sessionList; *sp;) {
		uint32_t kickTime = NetSession_get_lastKeepAlive(&(*sp)->net) + 180000;
		if(currentTime > kickTime) { // this filters the RFC-1149 user
			*sp = master_disconnect(*sp);
		} else {
			if(kickTime < *nextTick)
				*nextTick = kickTime;
			struct MasterSession *session = *sp;
			for(uint32_t i = 0; i < session->resend.count; ++i) {
				if(session->resend.index[i].shouldSend && currentTime - session->resend.index[i].timeStamp >= NET_RESEND_DELAY) {
					net_send_internal(&ctx->net, &session->net, session->resend.data[session->resend.index[i].data].data, session->resend.data[session->resend.index[i].data].len, session->resend.index[i].encrypt);
					while(currentTime - session->resend.index[i].timeStamp >= NET_RESEND_DELAY)
						session->resend.index[i].timeStamp += NET_RESEND_DELAY;
				}
				if(session->resend.index[i].timeStamp < *nextTick)
					*nextTick = session->resend.index[i].timeStamp;
			}
			sp = &(*sp)->next;
		}
	}
}

static uint32_t master_getNextRequestId(struct MasterSession *session) {
	++session->lastSentRequestId;
	return (session->lastSentRequestId & 63) | session->epoch;
}

static bool master_handle_ack(struct MasterSession *session, MessageType *messageType_out, uint8_t *serialType_out, const struct MessageReceivedAcknowledge *ack) {
	uint32_t requestId = ack->base.responseId;
	for(uint32_t i = 0; i < session->resend.count; ++i) {
		if(requestId == session->resend.index[i].requestId) {
			--session->resend.count;
			uint32_t data = session->resend.index[i].data;
			session->resend.index[i] = session->resend.index[session->resend.count];
			session->resend.index[session->resend.count].data = data;
			*messageType_out = session->resend.data[data].messageType;
			*serialType_out = session->resend.data[data].serialType;
			return 1;
		}
	}
	return 0;
}

static void master_net_send_reliable(struct NetContext *ctx, struct MasterSession *session, const uint8_t *buf, uint32_t len, uint32_t requestId, MessageType messageType, uint8_t serialType, bool shouldSend, bool encrypt) {
	if(session->resend.count < MASTER_WINDOW_SIZE) {
		struct MasterResendSparsePtr *p = &session->resend.index[session->resend.count++];
		p->shouldSend = shouldSend;
		p->encrypt = encrypt;
		p->timeStamp = net_time();
		p->requestId = requestId;
		session->resend.data[p->data].len = len;
		session->resend.data[p->data].messageType = messageType;
		session->resend.data[p->data].serialType = serialType;
		memcpy(session->resend.data[p->data].data, buf, len);
	} else {
		uprintf("RESEND BUFFER FULL\n");
	}
	if(shouldSend)
		net_send_internal(ctx, &session->net, buf, len, encrypt);
}

static struct MasterServerReliableRequestProxy get_request_info(const uint8_t *data, const uint8_t *data_end, struct PacketContext version) {
	struct MasterServerReliableRequestProxy out = {~0};
	pkt_read(&(struct SerializeHeader){0}, &data, data_end, version);
	pkt_read(&out, &data, data_end, version);
	return out;
}

static void master_send(struct NetContext *ctx, struct MasterSession *session, MessageType type, const uint8_t *data, const uint8_t *data_end, bool reliable) {
	uint8_t buf[data_end + 64 - data], *buf_end = buf;
	struct UnconnectedMessage header = {
		.type = type,
		.protocolVersion = session->net.version.protocolVersion,
	};
	if(data_end - data <= 414) {
		bool res = pkt_write_c(&buf_end, endof(buf), PV_LEGACY_DEFAULT, NetPacketHeader, {
			.property = PacketProperty_UnconnectedMessage,
			.connectionNumber = 0,
			.isFragmented = false,
			.unconnectedMessage = header,
		}) && pkt_write_bytes(data, &buf_end, endof(buf), PV_LEGACY_DEFAULT, data_end - data);
		if(!res)
			return;
		if(reliable) {
			struct MasterServerReliableRequestProxy request = get_request_info(data, data_end, PV_LEGACY_DEFAULT);
			master_net_send_reliable(ctx, session, buf, buf_end - buf, request.value.requestId, type, request.type, true, type != MessageType_HandshakeMessage);
		} else {
			net_send_internal(ctx, &session->net, buf, buf_end - buf, type != MessageType_HandshakeMessage);
		}
		return;
	}
	struct MasterServerReliableRequestProxy request = get_request_info(data, data_end, PV_LEGACY_DEFAULT);
	if(!(pkt_write(&header, &buf_end, endof(buf), PV_LEGACY_DEFAULT) &&
	     pkt_write_bytes(data, &buf_end, endof(buf), PV_LEGACY_DEFAULT, data_end - data)))
		return;
	struct MultipartMessageProxy mpp = {
		.value = {
			.multipartMessageId = request.value.requestId,
			.offset = 0,
			.length = 384,
			.totalLength = buf_end - buf,
		},
	};
	switch(type) {
		case MessageType_UserMessage: mpp.type = UserMessageType_MultipartMessage; break;
		case MessageType_HandshakeMessage: mpp.type = HandshakeMessageType_MultipartMessage; break;
		default: uprintf("Bad MessageType in master_send()\b"); return;
	}
	const uint8_t *buf_it = buf;
	do {
		mpp.value.base.requestId = master_getNextRequestId(session);
		mpp.value.offset = buf_it - buf;
		if(buf_end - buf_it < mpp.value.length)
			mpp.value.length = buf_end - buf_it;
		uint8_t mpbuf[512], *mpbuf_end = mpbuf;
		memcpy(mpp.value.data, buf_it, mpp.value.length);
		bool res = pkt_write_c(&mpbuf_end, endof(mpbuf), PV_LEGACY_DEFAULT, NetPacketHeader, {
			.property = PacketProperty_UnconnectedMessage,
			.connectionNumber = 0,
			.isFragmented = false,
			.unconnectedMessage = {
				.type = type,
				.protocolVersion = session->net.version.protocolVersion,
			},
		}) && MASTER_SERIALIZE(&mpp, &mpbuf_end, endof(mpbuf));
		(void)res;
		master_net_send_reliable(ctx, session, mpbuf, mpbuf_end - mpbuf, mpp.value.base.requestId, type, request.type, true, type != MessageType_HandshakeMessage);
		buf_it += mpp.value.length;
	} while(buf_it < buf_end);
	master_net_send_reliable(ctx, session, NULL, 0, mpp.value.multipartMessageId, type, request.type, false, false);
}

static void master_send_ack(struct Context *ctx, struct MasterSession *session, MessageType type, uint32_t requestId) {
	struct MessageReceivedAcknowledgeProxy r_ack = {
		.type = (type == MessageType_UserMessage) ? UserMessageType_MessageReceivedAcknowledge : HandshakeMessageType_MessageReceivedAcknowledge,
		.value = {
			.base = {
				.responseId = requestId,
			},
			.messageHandled = 1,
		},
	};
	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_ack, &resp_end, endof(resp)))
		return;
	master_send(&ctx->net, session, type, resp, resp_end, false);
}

static void handle_ClientHelloRequest(struct Context *ctx, struct MasterSession *session, const struct ClientHelloRequest *req) {
	if(session->handshakeStep != 255) {
		if(net_time() - NetSession_get_lastKeepAlive(&session->net) < 5000) // 5 second timeout to prevent clients from getting "locked out" if their previous session hasn't closed or timed out yet
			return;
		net_session_reset(&ctx->net, &session->net); // security or something idk
		session->resend.count = 0;
	}
	session->epoch = req->base.requestId & 0xff000000;
	memcpy(session->net.clientRandom, req->random, 32);
	struct HandshakeMessage r_hello = {
		.type = HandshakeMessageType_HelloVerifyRequest,
		.helloVerifyRequest = {
			.base = {
				.requestId = 0,
				.responseId = req->base.requestId,
			},
		},
	};
	memcpy(r_hello.helloVerifyRequest.cookie, NetSession_get_cookie(&session->net), sizeof(r_hello.helloVerifyRequest.cookie));
	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_hello, &resp_end, endof(resp)))
		return;
	master_send(&ctx->net, session, MessageType_HandshakeMessage, resp, resp_end, true);
	session->handshakeStep = HandshakeMessageType_ClientHelloRequest;
}

static void handle_ClientHelloWithCookieRequest(struct Context *ctx, struct MasterSession *session, const struct ClientHelloWithCookieRequest *req) {
	master_send_ack(ctx, session, MessageType_HandshakeMessage, req->base.requestId);
	if(session->handshakeStep != HandshakeMessageType_ClientHelloRequest)
		return;
	if(memcmp(req->cookie, NetSession_get_cookie(&session->net), 32) != 0)
		return;
	if(memcmp(req->random, session->net.clientRandom, 32) != 0)
		return;
	session->ClientHelloWithCookieRequest_requestId = req->base.requestId; // don't @ me   -rc

	struct HandshakeMessage r_cert = {
		.type = HandshakeMessageType_ServerCertificateRequest,
		.serverCertificateRequest = {
			.base = {
				.requestId = master_getNextRequestId(session),
				.responseId = req->certificateResponseId,
			},
			.certificateCount = 0,
		},
	};
	for(const mbedtls_x509_crt *it = ctx->cert; it; it = it->MBEDTLS_PRIVATE(next)) {
		r_cert.serverCertificateRequest.certificateList[r_cert.serverCertificateRequest.certificateCount].length = it->MBEDTLS_PRIVATE(raw).MBEDTLS_PRIVATE(len);
		memcpy(r_cert.serverCertificateRequest.certificateList[r_cert.serverCertificateRequest.certificateCount].data, it->MBEDTLS_PRIVATE(raw).MBEDTLS_PRIVATE(p), r_cert.serverCertificateRequest.certificateList[r_cert.serverCertificateRequest.certificateCount].length);
		++r_cert.serverCertificateRequest.certificateCount;
	}
	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_cert, &resp_end, endof(resp)))
		return;
	master_send(&ctx->net, session, MessageType_HandshakeMessage, resp, resp_end, true);
	session->handshakeStep = HandshakeMessageType_ClientHelloWithCookieRequest;
}

static void handle_ServerCertificateRequest_ack(struct Context *ctx, struct MasterSession *session) {
	if(session->handshakeStep != HandshakeMessageType_ClientHelloWithCookieRequest)
		return;
	struct HandshakeMessage r_hello = {
		.type = HandshakeMessageType_ServerHelloRequest,
		.serverHelloRequest = {
			.base = {
				.requestId = master_getNextRequestId(session),
				.responseId = session->ClientHelloWithCookieRequest_requestId,
			},
			.publicKey = {
				.length = sizeof(r_hello.serverHelloRequest.publicKey.data),
			},
		},
	};
	memcpy(r_hello.serverHelloRequest.random, NetKeypair_get_random(&session->net.keys), sizeof(r_hello.serverHelloRequest.random));
	if(NetKeypair_write_key(&session->net.keys, &ctx->net, r_hello.serverHelloRequest.publicKey.data, &r_hello.serverHelloRequest.publicKey.length))
		return;
	{
		uint8_t sig[r_hello.serverHelloRequest.publicKey.length + 64];
		memcpy(sig, session->net.clientRandom, 32);
		memcpy(&sig[32], NetKeypair_get_random(&session->net.keys), 32);
		memcpy(&sig[64], r_hello.serverHelloRequest.publicKey.data, r_hello.serverHelloRequest.publicKey.length);
		NetSession_signature(&session->net, &ctx->net, ctx->key, sig, sizeof(sig), &r_hello.serverHelloRequest.signature);
	}

	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_hello, &resp_end, endof(resp)))
		return;
	master_send(&ctx->net, session, MessageType_HandshakeMessage, resp, resp_end, true);
	session->handshakeStep = HandshakeMessageType_ServerCertificateRequest;
}

static void handle_ClientKeyExchangeRequest(struct Context *ctx, struct MasterSession *session, const struct ClientKeyExchangeRequest *req) {
	master_send_ack(ctx, session, MessageType_HandshakeMessage, req->base.requestId);
	if(session->handshakeStep != HandshakeMessageType_ServerCertificateRequest)
		return;
	if(NetSession_set_clientPublicKey(&session->net, &ctx->net, &req->clientPublicKey))
		return;
	struct HandshakeMessage r_spec = {
		.type = HandshakeMessageType_ChangeCipherSpecRequest,
		.changeCipherSpecRequest = {
			.base = {
				.requestId = master_getNextRequestId(session),
				.responseId = req->base.requestId,
			},
		},
	};
	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_spec, &resp_end, endof(resp)))
		return;
	master_send(&ctx->net, session, MessageType_HandshakeMessage, resp, resp_end, true);
	session->handshakeStep = HandshakeMessageType_ClientKeyExchangeRequest;
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
	master_send(&ctx->net, session, MessageType_UserMessage, resp, resp_end, true);
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

static void handle_ConnectToServerRequest(struct Context *ctx, struct MasterSession *session, const struct ConnectToServerRequest *req) {
	master_send_ack(ctx, session, MessageType_UserMessage, req->base.base.requestId);
	// TODO: deduplicate this request
	struct UserMessage r_conn = {
		.type = UserMessageType_ConnectToServerResponse,
		.connectToServerResponse = {
			.base = {
				.requestId = master_getNextRequestId(session),
				.responseId = req->base.base.requestId,
			},
			.result = ConnectToServerResponse_Result_UnknownError,
		},
	};
	if(req->configuration.maxPlayerCount >= 127) { // connection IDs are 7 bits, with ID `127` reserved for broadcast packets and `0` for local
		r_conn.connectToServerResponse.result = ConnectToServerResponse_Result_InvalidCode;
		goto send;
	}
	struct String managerId = req->base.userId;
	struct SS addr = *NetSession_get_addr(&session->net);
	struct NetSession *isession;

	struct RoomHandle room;
	struct WireRoomHandle handle;
	if(req->code == StringToServerCode(NULL, 0)) {
		if(req->configuration.gameplayServerMode == GameplayServerMode_Countdown) {
			r_conn.connectToServerResponse.result = ConnectToServerResponse_Result_NoAvailableDedicatedServers; // Quick Play not yet available
			goto send;
		}
		r_conn.connectToServerResponse.configuration = req->configuration;
		#ifdef FORCE_MASSIVE_LOBBIES
		r_conn.connectToServerResponse.configuration.maxPlayerCount = 126;
		r_conn.connectToServerResponse.configuration.songSelectionMode = SongSelectionMode_Vote;
		#endif
		if(pool_request_room(&room, &handle, managerId, r_conn.connectToServerResponse.configuration)) {
			r_conn.connectToServerResponse.result = ConnectToServerResponse_Result_NoAvailableDedicatedServers;
			goto send;
		}
		r_conn.connectToServerResponse.code = pool_room_code(room);
	} else {
		if(pool_find_room(req->code, &room, &handle)) {
			r_conn.connectToServerResponse.result = ConnectToServerResponse_Result_InvalidCode;
			goto send;
		}
		managerId = wire_room_get_managerId(handle);
		r_conn.connectToServerResponse.configuration = wire_room_get_configuration(handle);

		if(wire_room_get_protocol(handle).protocolVersion != session->net.version.protocolVersion) {
			r_conn.connectToServerResponse.result = ConnectToServerResponse_Result_VersionMismatch;
			goto send;
		}
		r_conn.connectToServerResponse.code = req->code;
	}
	/*struct BitMask128 customs = get_mask("custom_levelpack_CustomLevels");
	req->selectionMask.songPacks.bloomFilter.d0 |= customs.d0;
	req->selectionMask.songPacks.bloomFilter.d1 |= customs.d1;*/
	{
		isession = TEMPwire_room_resolve_session(handle, addr, req->secret, req->base.userId, (struct ExString){req->base.userName, 0}, session->net.version);
		if(!isession) { // TODO: the room should close implicitly if the first user to connect fails
			r_conn.connectToServerResponse.result = ConnectToServerResponse_Result_UnknownError;
			goto send;
		}
		struct NetContext *net = TEMPwire_block_get_net(handle.block);
		net_lock(net);
		memcpy(isession->clientRandom, req->base.random, 32);
		memcpy(r_conn.connectToServerResponse.random, NetKeypair_get_random(&isession->keys), 32);
		r_conn.connectToServerResponse.publicKey.length = sizeof(r_conn.connectToServerResponse.publicKey.data);
		if(NetKeypair_write_key(&isession->keys, net, r_conn.connectToServerResponse.publicKey.data, &r_conn.connectToServerResponse.publicKey.length)) {
			net_unlock(net);
			r_conn.connectToServerResponse.result = ConnectToServerResponse_Result_UnknownError;
			goto send;
		}
		if(NetSession_set_clientPublicKey(isession, net, &req->base.publicKey)) {
			net_unlock(net);
			r_conn.connectToServerResponse.result = ConnectToServerResponse_Result_UnknownError;
			goto send;
		}
		net_unlock(net);
		r_conn.connectToServerResponse.userId.isNull = 0;
		r_conn.connectToServerResponse.userId.length = sprintf(r_conn.connectToServerResponse.userId.data, "dtxJlHm56k6ZXcnxhbyfiA");
		r_conn.connectToServerResponse.userName.length = 0;
		r_conn.connectToServerResponse.secret = req->secret;
		r_conn.connectToServerResponse.selectionMask = req->selectionMask;
		r_conn.connectToServerResponse.flags = 3;
		r_conn.connectToServerResponse.remoteEndPoint = wire_block_get_endpoint(handle.block, addr.ss.ss_family != AF_INET6 || memcmp(addr.in6.sin6_addr.s6_addr, (const uint8_t[]){0,0,0,0,0,0,0,0,0,0,255,255}, 12) == 0);
		r_conn.connectToServerResponse.managerId = managerId;
		r_conn.connectToServerResponse.result = ConnectToServerResponse_Result_Success;
		char scode[8];
		uprintf("Sending player to room `%s`\n", ServerCodeToString(scode, r_conn.connectToServerResponse.code));
	}
	send:;
	uint8_t resp[65536], *resp_end = resp;
	if(!MASTER_SERIALIZE(&r_conn, &resp_end, endof(resp)))
		return;
	master_send(&ctx->net, session, MessageType_UserMessage, resp, resp_end, true);
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
				session->net.version.protocolVersion = header.protocolVersion;
			struct UserMessage message = {~0};
			pkt_read(&message, &sub, &sub[serial.length], session->net.version);
			if(check_length("BAD USER MESSAGE LENGTH", sub, data, serial.length, session->net.version))
				continue;
			switch(message.type) {
				case UserMessageType_AuthenticateUserRequest: handle_AuthenticateUserRequest(ctx, session, &message.authenticateUserRequest); break;
				case UserMessageType_AuthenticateUserResponse: uprintf("BAD TYPE: UserMessageType_AuthenticateUserResponse\n"); break;
				case UserMessageType_ConnectToServerResponse: uprintf("BAD TYPE: UserMessageType_ConnectToServerResponse\n"); break;
				case UserMessageType_ConnectToServerRequest: handle_ConnectToServerRequest(ctx, session, &message.connectToServerRequest); break;
				case UserMessageType_MessageReceivedAcknowledge: master_handle_ack(session, &(MessageType){0}, &(uint8_t){0}, &message.messageReceivedAcknowledge); break;
				case UserMessageType_MultipartMessage: handle_MultipartMessage(ctx, session, &message.multipartMessage); break;
				case UserMessageType_SessionKeepaliveMessage: break;
				case UserMessageType_GetPublicServersRequest: uprintf("UserMessageType_GetPublicServersRequest not implemented\n"); abort();
				case UserMessageType_GetPublicServersResponse: uprintf("UserMessageType_GetPublicServersResponse not implemented\n"); abort();
				default: uprintf("BAD USER MESSAGE TYPE: %hhu\n", message.type);
			}
		} else if(header.type == MessageType_HandshakeMessage) {
			if(header.protocolVersion > session->net.version.protocolVersion)
				session->net.version.protocolVersion = header.protocolVersion;
			struct HandshakeMessage message = {~0};
			pkt_read(&message, &sub, &sub[serial.length], session->net.version);
			if(check_length("BAD HANDSHAKE MESSAGE LENGTH", sub, data, serial.length, session->net.version))
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
					MessageType messageType;
					uint8_t serialType;
					if(master_handle_ack(session, &messageType, &serialType, &message.messageReceivedAcknowledge)) {
						if(messageType == MessageType_HandshakeMessage && serialType == HandshakeMessageType_ServerCertificateRequest)
							handle_ServerCertificateRequest_ack(ctx, session);
					}
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

thread_return_t master_handler(struct Context *ctx) {
	net_lock(&ctx->net);
	uprintf("Started\n");
	uint8_t buf[262144];
	memset(buf, 0, sizeof(buf));
	uint32_t len;
	struct MasterSession *session;
	const uint8_t *pkt;
	while((len = net_recv(&ctx->net, buf, sizeof(buf), (struct NetSession**)&session, &pkt, NULL))) {
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

static thread_t master_thread = 0;
static struct Context ctx = {{-1}, NULL, NULL, NULL};
bool master_init(const mbedtls_x509_crt *cert, const mbedtls_pk_context *key, uint16_t port) {
	{
		uint_fast8_t count = 0;
		for(const mbedtls_x509_crt *it = cert; it; it = it->MBEDTLS_PRIVATE(next), ++count) {
			if(it->MBEDTLS_PRIVATE(raw).MBEDTLS_PRIVATE(len) > 4096) {
				uprintf("Host certificate too large\n");
				return 1;
			}
		}
		if(count > lengthof(((struct ServerCertificateRequest*)NULL)->certificateList)) {
			uprintf("Host certificate chain too long\n");
			return 1;
		}
	}
	ctx.cert = cert;
	ctx.key = key;
	if(net_init(&ctx.net, port)) {
		uprintf("net_init() failed\n");
		return 1;
	}
	ctx.net.user = &ctx;
	ctx.net.onResolve = master_onResolve;
	ctx.net.onResend = master_onResend;
	return thread_create(&master_thread, master_handler, &ctx);
}

void master_cleanup() {
	if(master_thread) {
		net_stop(&ctx.net);
		uprintf("Stopping\n");
		thread_join(master_thread);
		master_thread = 0;
	}
	while(ctx.sessionList)
		ctx.sessionList = master_disconnect(ctx.sessionList);
	net_cleanup(&ctx.net);
}
