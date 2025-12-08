#include "master_internal.h"
#include <stdlib.h>
#include <assert.h>

struct MasterSession *MasterContext_lookup(struct MasterContext *ctx, struct SS addr) {
	for(struct MasterSession *session = ctx->sessionList; session; session = session->next)
		if(SS_equal(&addr, NetSession_get_addr(&session->net)))
			return session;
	return NULL;
}

struct NetSession *MasterContext_onResolve(struct MasterContext *ctx, struct NetContext *net, struct SS addr, const uint8_t packet[static 1536], uint32_t packet_len, uint8_t out[static 1536], uint32_t *out_len) {
	if(packet_len == 0)
		return NULL;
	struct MasterSession *session = MasterContext_lookup(ctx, addr);
	if(session != NULL) {
		*out_len = NetSession_decrypt(&session->net, packet, packet_len, out);
		return &session->net;
	}
	if(*packet != 0 && *packet != MBEDTLS_SSL_MSG_HANDSHAKE && *packet != EENET_CONNECT_BYTE)
		return NULL;
	session = malloc(sizeof(struct MasterSession));
	if(session == NULL) {
		uprintf("alloc error\n");
		return NULL;
	}
	NetSession_init(&session->net, net, addr, &ctx->config, 0);
	session->lastSentRequestId = 0;
	session->handshake.step = HandshakeMessageType_ClientHelloRequest;
	session->resend.set = (struct Counter64){};
	session->multipartList = NULL;
	session->enet = NULL;
	if(*packet != 0) {
		session->enet = eenet_init();
		eenet_attach(session->enet, net, &session->net);
	}
	session->next = ctx->sessionList;
	ctx->sessionList = session;

	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(&addr, addrstr);
	uprintf("connect %s\n", addrstr);

	*out_len = NetSession_decrypt(&session->net, packet, packet_len, out);
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
	if(session->enet)
		eenet_free(session->enet);
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
				net_send_internal(net, &session->net, slot->data, slot->length, slot->encrypt ? EncryptMode_BGNet : EncryptMode_None);
				slot->lastSend = currentTime + NET_RESEND_DELAY - (currentTime - slot->lastSend) % NET_RESEND_DELAY;
			}
			if(slot->lastSend - currentTime < nextTick)
				nextTick = slot->lastSend - currentTime;
		}
		if(session->enet != NULL)
			eenet_tick(session->enet);
		if(unresponsive) // avoids spamming resends for the full 3 minute timeout if the client isn't handling them
			*sp = master_disconnect(*sp);
		else
			sp = &(*sp)->next;
	}
	return nextTick;
}

static uint32_t MasterSession_prevRequestId(const struct MasterSession *const session) {
	return (session->lastSentRequestId & 63) | session->epoch;
}

uint32_t MasterSession_nextRequestId(struct MasterSession *const session) {
	++session->lastSentRequestId;
	return MasterSession_prevRequestId(session);
}

// return is valid until the next call to `master_send()`
static const struct MasterPacket *master_handle_ack(struct MasterSession *session, const struct MessageReceivedAcknowledge *ack) {
	const uint32_t requestId = ack->base.responseId;
	struct Counter64 iter = session->resend.set;
	for(uint32_t i; (i = Counter64_clear_next(&iter)) != COUNTER64_INVALID;) {
		if(requestId != session->resend.requestIds[i])
			continue;
		Counter64_clear(&session->resend.set, i);
		return &session->resend.slots[i];
	}
	return NULL;
}

static void master_send_internal(struct NetContext *net, struct MasterSession *session, const uint8_t *buf, uint16_t length, bool encrypt) {
	uint32_t i = Counter64_set_next(&session->resend.set);
	if(i != COUNTER64_INVALID) {
		struct MasterPacket *slot = &session->resend.slots[i];
		slot->firstSend = slot->lastSend = net_time();
		slot->length = length;
		slot->encrypt = encrypt;
		memcpy(slot->data, buf, length);
		session->resend.requestIds[i] = MasterSession_prevRequestId(session);
	} else {
		uprintf("RESEND BUFFER FULL\n");
		net_send_internal(net, &session->net, buf, length, encrypt ? EncryptMode_BGNet : EncryptMode_None);
	}
}

#define master_send(net, session, ...) MasterSession_send(net, session, _Generic(*(__VA_ARGS__), \
	struct UserMessage: MessageType_UserMessage, \
	struct GameLiftMessage: MessageType_GameLiftMessage, \
	struct HandshakeMessage: MessageType_HandshakeMessage), __VA_ARGS__)
uint32_t MasterSession_send(struct NetContext *const net, struct MasterSession *const session, const MessageType type, const void *const message) {
	uint8_t resp[65536], *resp_end = resp;
	bool res = pkt_write_c(&resp_end, endof(resp), session->net.version, NetPacketHeader, {
		.property = PacketProperty_UnconnectedMessage,
		.unconnectedMessage = {
			.type = type,
			.protocolVersion = session->net.version.protocolVersion,
		},
	});
	switch(type) {
		case MessageType_UserMessage: res = res && pkt_serialize((const struct UserMessage*)message, &resp_end, endof(resp), session->net.version); break;
		case MessageType_GameLiftMessage: res = res && pkt_serialize((const struct GameLiftMessage*)message, &resp_end, endof(resp), session->net.version); break;
		case MessageType_HandshakeMessage: res = res && pkt_serialize((const struct HandshakeMessage*)message, &resp_end, endof(resp), session->net.version); break;
		default: uprintf("Bad MessageType in master_send()\n"); return 0;
	}
	if(!res)
		return 0;
	if(resp_end - resp <= 412) { // TODO: use `session->mtu` here?
		if(type == MessageType_HandshakeMessage && ((const struct HandshakeMessage*)message)->type == HandshakeMessageType_HelloVerifyRequest)
			net_send_internal(net, &session->net, resp, (uint16_t)(resp_end - resp), EncryptMode_None);
		else
			master_send_internal(net, session, resp, (uint16_t)(resp_end - resp), type != MessageType_HandshakeMessage);
		return 1;
	}
	struct MultipartMessageProxy mmp = {
		.value = {
			.multipartMessageId = MasterSession_prevRequestId(session),
			.length = 384,
			.totalLength = (uint32_t)(resp_end - &resp[1]), // TODO: `static_assert()` offset of serialized `NetPacketHeader.unconnectedMessage` to warn on potential ABI breaks
		},
	};
	switch(type) {
		case MessageType_UserMessage: mmp.type = UserMessageType_MultipartMessage; break;
		case MessageType_GameLiftMessage: mmp.type = GameLiftMessageType_MultipartMessage; break;
		case MessageType_HandshakeMessage: mmp.type = HandshakeMessageType_MultipartMessage; break;
	}
	uint32_t partCount = 0;
	do {
		mmp.value.base.requestId = MasterSession_nextRequestId(session);
		if(mmp.value.length > mmp.value.totalLength - mmp.value.offset)
			mmp.value.length = mmp.value.totalLength - mmp.value.offset;
		uint8_t part[512], *part_end = part;
		memcpy(mmp.value.data, &resp[1 + mmp.value.offset], mmp.value.length);
		bool res = pkt_write_c(&part_end, endof(part), session->net.version, NetPacketHeader, {
			.property = PacketProperty_UnconnectedMessage,
			.unconnectedMessage = {
				.type = type,
				.protocolVersion = session->net.version.protocolVersion,
			},
		}) && pkt_serialize(&mmp, &part_end, endof(part), session->net.version);
		if(res)
			master_send_internal(net, session, part, (uint16_t)(part_end - part), type != MessageType_HandshakeMessage);
		++partCount;
		mmp.value.offset += mmp.value.length;
	} while(mmp.value.offset < mmp.value.totalLength);
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
	bool res = pkt_write_c(&resp_end, endof(resp), session->net.version, NetPacketHeader, {
		.property = PacketProperty_UnconnectedMessage,
		.unconnectedMessage = {
			.type = type,
			.protocolVersion = session->net.version.protocolVersion,
		},
	}) && pkt_serialize((&(struct MessageReceivedAcknowledgeProxy){ // TODO: gl
		.type = ackType,
		.value = {
			.base.responseId = requestId,
			.messageHandled = true,
		},
	}), &resp_end, endof(resp), session->net.version);
	if(res)
		net_send_internal(net, &session->net, resp, (uint32_t)(resp_end - resp), (type != MessageType_HandshakeMessage) ? EncryptMode_BGNet : EncryptMode_None);
}

static inline bool InitializeConnection(struct NetContext *net, struct MasterSession *session, const mbedtls_ssl_config *config, const struct ClientHelloRequest *req) {
	if(session->handshake.step != HandshakeMessageType_ClientHelloRequest) {
		// 5 second timeout to prevent clients from getting "locked out" if their previous session hasn't closed or timed out yet
		if(net_time() - NetSession_get_lastKeepAlive(&session->net) < 5000)
			return session->handshake.step == HandshakeMessageType_ClientHelloWithCookieRequest;
		struct SS addr = *NetSession_get_addr(&session->net);
		NetSession_free(&session->net);
		NetSession_init(&session->net, net, addr, config, 0); // security or something idk
		session->resend.set = (struct Counter64){};
	}
	session->epoch = req->base.requestId & 0xff000000;
	session->net.clientRandom = req->random;
	session->handshake.step = HandshakeMessageType_ClientHelloWithCookieRequest;
	return true;
}

static void handle_ClientHelloRequest(struct NetContext *net, struct MasterSession *session, const mbedtls_ssl_config *config, const struct ClientHelloRequest *req) {
	if(!InitializeConnection(net, session, config, req))
		return;
	master_send(net, session, &(const struct HandshakeMessage){
		.type = HandshakeMessageType_HelloVerifyRequest,
		.helloVerifyRequest = {
			.base = {
				.requestId = MasterSession_nextRequestId(session),
				.responseId = req->base.requestId,
			},
			.cookie = *NetSession_get_cookie(&session->net),
		},
	});
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
			.requestId = MasterSession_nextRequestId(session),
			.responseId = req->certificateResponseId,
		},
	};
	for(const mbedtls_x509_crt *it = &ctx->cert; it != NULL; it = it->next) {
		r_cert.serverCertificateRequest.certificateList[r_cert.serverCertificateRequest.certificateCount].length = (uint32_t)it->raw.len;
		memcpy(r_cert.serverCertificateRequest.certificateList[r_cert.serverCertificateRequest.certificateCount].data, it->raw.p, r_cert.serverCertificateRequest.certificateList[r_cert.serverCertificateRequest.certificateCount].length);
		++r_cert.serverCertificateRequest.certificateCount;
	}

	const uint32_t packetCount = master_send(net, session, &r_cert);
	if(!packetCount)
		return;
	session->handshake = (struct HandshakeState){
		.certificateRequestId = r_cert.serverCertificateRequest.base.requestId,
		.helloResponseId = req->base.requestId,
		.certificateOutboundCount = packetCount,
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
				.requestId = MasterSession_nextRequestId(session),
				.responseId = session->handshake.helloResponseId,
			},
			.random = *NetKeypair_get_random(&session->net.keys),
		},
	};
	if(NetKeypair_write_key(&session->net.keys, net, &r_hello.serverHelloRequest.publicKey))
		return;
	if(mbedtls_pk_get_type(&ctx->key) != MBEDTLS_PK_NONE)
		NetSession_signature(&session->net, net, &ctx->key, &r_hello.serverHelloRequest.signature);
	master_send(net, session, &r_hello);
	session->handshake.step = HandshakeMessageType_ClientKeyExchangeRequest;
}

static void handle_ClientKeyExchangeRequest(struct NetContext *net, struct MasterSession *session, const struct ClientKeyExchangeRequest *req) {
	master_send_ack(net, session, MessageType_HandshakeMessage, req->base.requestId);
	if(session->handshake.step != HandshakeMessageType_ClientKeyExchangeRequest)
		return;
	if(NetSession_set_remotePublicKey(&session->net, net, &req->clientPublicKey, false))
		return;
	master_send(net, session, &(const struct HandshakeMessage){
		.type = HandshakeMessageType_ChangeCipherSpecRequest,
		.changeCipherSpecRequest.base = {
			.requestId = MasterSession_nextRequestId(session),
			.responseId = req->base.requestId,
		},
	});
	session->handshake.step = HandshakeMessageType_ChangeCipherSpecRequest;
}

static void handle_BaseAuthenticate(struct NetContext *net, struct MasterSession *session, struct BaseMasterServerReliableResponse req, bool gamelift) {
	static_assert((uint32_t)GameLiftMessageType_AuthenticateUserResponse == (uint32_t)UserMessageType_AuthenticateUserResponse, "ABI break");
	master_send_ack(net, session, gamelift ? MessageType_GameLiftMessage : MessageType_UserMessage, req.requestId);
	const struct AuthenticateUserResponse r_auth = {
		.base = {
			.requestId = MasterSession_nextRequestId(session),
			.responseId = req.requestId,
		},
		.result = AuthenticateUserResponse_Result_Success,
	};
	if(gamelift)
		master_send(net, session, &(const struct GameLiftMessage){
			.type = GameLiftMessageType_AuthenticateUserResponse,
			.authenticateUserResponse = r_auth,
		});
	else
		master_send(net, session, &(const struct UserMessage){
			.type = UserMessageType_AuthenticateUserResponse,
			.authenticateUserResponse = r_auth,
		});
}

static void handle_AuthenticateUserRequest(struct NetContext *net, struct MasterSession *session, const struct AuthenticateUserRequest *req) {
	handle_BaseAuthenticate(net, session, req->base, false);
}

static bool handle_AuthenticateGameLiftUserRequest(struct NetContext *net, struct MasterSession *session, const struct AuthenticateGameLiftUserRequest *req, struct GraphAuthToken *auth_out) {
	handle_BaseAuthenticate(net, session, req->base, true);
	if(auth_out == NULL)
		return false;
	*auth_out = (struct GraphAuthToken){
		.base = {
			.userId = req->userId,
			.userName = req->userName,
			.playerSessionId = req->playerSessionId,
		},
	};
	return true;
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

static bool handle_MultipartMessage(struct MasterContext *ctx, struct NetContext *net, struct MasterSession *session, const struct MultipartMessage *msg, struct GraphAuthToken *auth_out) {
	if(!msg->totalLength) {
		uprintf("INVALID MULTIPART LENGTH\n");
		return false;
	}
	struct MasterMultipartList **multipart = &session->multipartList;
	for(; *multipart; multipart = &(*multipart)->next) {
		if((*multipart)->id == msg->multipartMessageId) {
			if((*multipart)->totalLength != msg->totalLength) {
				uprintf("BAD MULTIPART LENGTH\n");
				return false;
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
		return false;
	}
	memcpy(&(*multipart)->data[msg->offset], msg->data, msg->length);
	if(++(*multipart)->count < (msg->totalLength + sizeof(msg->data) - 1) / sizeof(msg->data))
		return false;
	const uint8_t *data = (*multipart)->data, *end = &(*multipart)->data[msg->totalLength];
	struct UnconnectedMessage header;
	const bool res = pkt_read(&header, &data, end, session->net.version) && MasterContext_handleMessage(ctx, net, session, header, data, end, auth_out);
	struct MasterMultipartList *e = *multipart;
	*multipart = (*multipart)->next;
	free(e);
	return res;
}

static void handle_ConnectToServerRequest(struct NetContext *const net, struct MasterSession *const session, const struct ConnectToServerRequest *const req) {
	master_send_ack(net, session, MessageType_UserMessage, req->base.base.requestId);
	LocalMasterContext_process_ConnectToServerRequest(net, session, req);
}

bool MasterContext_handleMessage(struct MasterContext *ctx, struct NetContext *net, struct MasterSession *session, struct UnconnectedMessage header, const uint8_t *data, const uint8_t *end, struct GraphAuthToken *auth_out) {
	while(data < end) {
		struct SerializeHeader serial;
		if(!pkt_read(&serial, &data, end, session->net.version))
			return false;
		const uint8_t *sub = data;
		data += serial.length;
		if(data > end) {
			uprintf("Invalid serial length: %u\n", serial.length);
			return false;
		}
		if((uint8_t)header.protocolVersion > session->net.version.protocolVersion)
			session->net.version.protocolVersion = (uint8_t)header.protocolVersion;
		if(header.type == MessageType_UserMessage) {
			session->net.version.direct = false;
			if(ctx != (struct MasterContext*)&LocalMasterContext_Instance)
				continue;
			struct UserMessage message = {.type = (UserMessageType)UINT32_C(0xffffffff)};
			pkt_read(&message, &sub, &sub[serial.length], session->net.version);
			if(pkt_debug("BAD USER MESSAGE LENGTH", data - serial.length, data, sub, session->net.version))
				continue;
			switch(message.type) {
				case UserMessageType_AuthenticateUserRequest: handle_AuthenticateUserRequest(net, session, &message.authenticateUserRequest); break;
				case UserMessageType_AuthenticateUserResponse: uprintf("BAD TYPE: UserMessageType_AuthenticateUserResponse\n"); break;
				case UserMessageType_ConnectToServerResponse: uprintf("BAD TYPE: UserMessageType_ConnectToServerResponse\n"); break;
				case UserMessageType_ConnectToServerRequest: handle_ConnectToServerRequest(net, session, &message.connectToServerRequest); break;
				case UserMessageType_MessageReceivedAcknowledge: master_handle_ack(session, &message.messageReceivedAcknowledge); break;
				case UserMessageType_MultipartMessage: {
					if(handle_MultipartMessage(ctx, net, session, &message.multipartMessage, auth_out))
						return true;
					break;
				}
				case UserMessageType_SessionKeepaliveMessage: break;
				case UserMessageType_GetPublicServersRequest: uprintf("UserMessageType_GetPublicServersRequest not implemented\n"); abort();
				case UserMessageType_GetPublicServersResponse: uprintf("BAD TYPE: UserMessageType_GetPublicServersResponse\n"); break;
				default: uprintf("BAD USER MESSAGE TYPE: %hhu\n", message.type);
			}
			continue;
		}
		if(header.type == MessageType_GameLiftMessage) {
			session->net.version.direct = true;
			if(ctx == (struct MasterContext*)&LocalMasterContext_Instance)
				continue;
			struct GameLiftMessage message = {.type = (GameLiftMessageType)UINT32_C(0xffffffff)};
			pkt_read(&message, &sub, &sub[serial.length], session->net.version);
			if(pkt_debug("BAD GAMELIFT MESSAGE LENGTH", data - serial.length, data, sub, session->net.version))
				continue;
			switch(message.type) {
				case GameLiftMessageType_AuthenticateGameLiftUserRequest: {
					if(handle_AuthenticateGameLiftUserRequest(net, session, &message.authenticateGameLiftUserRequest, auth_out))
						return true;
					break;
				}
				case GameLiftMessageType_AuthenticateUserResponse: uprintf("BAD TYPE: GameLiftMessageType_AuthenticateUserResponse\n"); break;
				case GameLiftMessageType_MessageReceivedAcknowledge: master_handle_ack(session, &message.messageReceivedAcknowledge); break;
				case GameLiftMessageType_MultipartMessage: {
					if(handle_MultipartMessage(ctx, net, session, &message.multipartMessage, auth_out))
						return true;
					break;
				}
				default: uprintf("BAD GAMELIFT MESSAGE TYPE: %hhu\n", message.type);
			}
			continue;
		}
		if(header.type == MessageType_HandshakeMessage) {
			struct HandshakeMessage message = {.type = (HandshakeMessageType)UINT32_C(0xffffffff)};
			pkt_read(&message, &sub, &sub[serial.length], session->net.version);
			if(pkt_debug("BAD HANDSHAKE MESSAGE LENGTH", data - serial.length, data, sub, session->net.version))
				continue;
			switch(message.type) {
				case HandshakeMessageType_ClientHelloRequest: handle_ClientHelloRequest(net, session, &ctx->config, &message.clientHelloRequest); break;
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
					if(!pkt_read(&readback, (const uint8_t*[]){sent->data}, &sent->data[sent->length], session->net.version))
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
	return false;
}

bool MasterContext_handle(struct MasterContext *ctx, struct NetContext *net, struct MasterSession *session, const uint8_t *data, const uint8_t *end, struct GraphAuthToken *auth_out) {
	if(session->enet != NULL) {
		session->net.version.direct = true;
		struct EENetPacket event = {0};
		for(eenet_handle(session->enet, data, end, &event); event.type != EENetPacketType_None; eenet_handle_next(session->enet, &event)) {
			static const uint8_t ident[] = {0x06,0x00,0x00,0x00,0x49,0x67,0x6e,0x43,0x6f,0x6e}; // String [length=6 data="IgnCon"]
			if(event.data_len <= sizeof(ident) || memcmp(event.data, ident, sizeof(ident)))
				continue;
			event.data += sizeof(ident);
			event.data_len -= sizeof(ident);
			event.type = EENetPacketType_ConnectMessage;
			if(pkt_read(&auth_out->base, (const uint8_t*[]){event.data}, &event.data[event.data_len], session->net.version)) {
				auth_out->enet = session->enet;
				auth_out->event = event;
				eenet_attach(session->enet, NULL, NULL);
				session->enet = NULL;
				return true;
			}
		}
		return false;
	}
	struct NetPacketHeader header;
	if(!pkt_read(&header, &data, end, session->net.version))
		return false;
	if(header.property == PacketProperty_UnconnectedMessage)
		return MasterContext_handleMessage(ctx, net, session, header.unconnectedMessage, data, end, auth_out);
	if(header.property != PacketProperty_Disconnect)
		uprintf("Unhandled property [%s]\n", reflect(PacketProperty, header.property));
	return false;
}

void MasterContext_init(struct MasterContext *const ctx, mbedtls_ctr_drbg_context *const ctr_drbg) {
	*ctx = (struct MasterContext){0};
	mbedtls_ssl_cookie_init(&ctx->cookie);
	mbedtls_ssl_config_init(&ctx->config);
	assert(mbedtls_ssl_cookie_setup(&ctx->cookie, mbedtls_ctr_drbg_random, ctr_drbg) == 0);
	assert(mbedtls_ssl_config_defaults(&ctx->config, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT) == 0);
	mbedtls_ssl_conf_rng(&ctx->config, mbedtls_ctr_drbg_random, ctr_drbg);
	mbedtls_ssl_conf_read_timeout(&ctx->config, 180000);
	mbedtls_ssl_conf_dtls_cookies(&ctx->config, mbedtls_ssl_cookie_write, mbedtls_ssl_cookie_check, &ctx->cookie);
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
	ctx->cert = *cert; // shallow copy
	ctx->key = *key; // shallow copy
	if(mbedtls_ssl_conf_own_cert(&ctx->config, &ctx->cert, &ctx->key))
		return true;
	mbedtls_ssl_conf_ca_chain(&ctx->config, cert->next, NULL);
	return false;
}

void MasterContext_cleanup(struct MasterContext *ctx) {
	while(ctx->sessionList)
		ctx->sessionList = master_disconnect(ctx->sessionList);
	mbedtls_ssl_config_free(&ctx->config);
	mbedtls_ssl_cookie_free(&ctx->cookie);
}
