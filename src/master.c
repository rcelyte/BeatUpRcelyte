#include "enum_reflection.h"
#include "instance.h"
#ifdef WINDOWS
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif
#include "debug.h"
#include <stdio.h>
#include <string.h>

#define lengthof(x) (sizeof(x)/sizeof(*x))

#define MASTER_SERIALIZE(pkt, mtype, stype, dtype, data) { \
	pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){ \
		.property = PacketProperty_UnconnectedMessage, \
		.connectionNumber = 0, \
		.isFragmented = 0, \
	}); \
	pkt_writeMessageHeader(pkt, (struct MessageHeader){ \
		.type = MessageType_##mtype, \
		.protocolVersion = 6, \
	}); \
	SERIALIZE_BODY(pkt, mtype##Type_##stype, dtype, data) \
}

struct MasterResendPacket {
	uint32_t len;
	uint8_t data[512];
};
struct MasterResendSparsePtr {
	_Bool shouldSend;
	_Bool encrypt;
	uint32_t timeStamp;
	uint32_t requestId;
	uint32_t data;
};
struct MasterResend {
	uint32_t count;
	struct MasterResendSparsePtr index[NET_WINDOW_SIZE];
	struct MasterResendPacket data[NET_WINDOW_SIZE];
};
struct MasterSession {
	struct NetSession net;
	struct MasterSession *next;
	uint32_t epoch;
	uint32_t lastSentRequestId;
	uint32_t ClientHelloWithCookieRequest_requestId;
	HandshakeMessageType handshakeStep;
	struct MasterResend resend;
};

struct Context {
	mbedtls_x509_crt *cert;
	mbedtls_pk_context *key;
	struct MasterSession *sessionList;
	struct NetContext net;
};

static struct NetSession *master_onResolve(struct Context *ctx, struct SS addr) {
	struct MasterSession *session = ctx->sessionList;
	for(; session; session = session->next)
		if(addrs_are_equal(&addr, NetSession_get_addr(&session->net)))
			return &session->net;
	session = malloc(sizeof(struct MasterSession));
	if(!session) {
		fprintf(stderr, "[MASTER] alloc error\n");
		return NULL;
	}
	if(net_session_init(&ctx->net, &session->net, addr)) {
		free(session);
		return NULL;
	}
	session->handshakeStep = 255;
	session->resend.count = 0;
	for(uint32_t i = 0; i < NET_WINDOW_SIZE; ++i)
		session->resend.index[i].data = i;
	session->next = ctx->sessionList;
	ctx->sessionList = session;

	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(&addr, addrstr);
	fprintf(stderr, "[MASTER] connect %s\n", addrstr);
	return &session->net;
}

static struct MasterSession *master_disconnect(struct MasterSession *session) {
	struct MasterSession *next = session->next;
	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(NetSession_get_addr(&session->net), addrstr);
	fprintf(stderr, "[MASTER] disconnect %s\n", addrstr);
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

static _Bool master_handle_ack(struct MasterSession *session, struct MessageHeader *message_out, struct SerializeHeader *serial_out, uint32_t requestId) {
	for(uint32_t i = 0; i < session->resend.count; ++i) {
		if(requestId == session->resend.index[i].requestId) {
			--session->resend.count;
			uint32_t data = session->resend.index[i].data;
			session->resend.index[i] = session->resend.index[session->resend.count];
			session->resend.index[session->resend.count].data = data;
			const uint8_t *msg = session->resend.data[data].data;
			*message_out = pkt_readMessageHeader(&msg);
			*serial_out = pkt_readSerializeHeader(&msg);
			return 1;
		}
	}
	return 0;
}

static void master_net_send_reliable(struct NetContext *ctx, struct MasterSession *session, const uint8_t *buf, uint32_t len, uint32_t requestId, _Bool shouldSend, _Bool encrypt) {
	if(session->resend.count < NET_WINDOW_SIZE) {
		struct MasterResendSparsePtr *p = &session->resend.index[session->resend.count++];
		p->shouldSend = shouldSend;
		p->encrypt = encrypt;
		p->timeStamp = net_time();
		p->requestId = requestId;
		session->resend.data[p->data].len = len;
		memcpy(session->resend.data[p->data].data, buf, len);
	} else {
		fprintf(stderr, "[MASTER] RESEND BUFFER FULL\n");
	}
	if(shouldSend)
		net_send_internal(ctx, &session->net, buf, len, encrypt);
}

static void master_send(struct NetContext *ctx, struct MasterSession *session, const uint8_t *buf, uint32_t len, _Bool reliable) {
	const uint8_t *data = buf, *end = &buf[len];
	pkt_readNetPacketHeader(&data);
	struct MessageHeader message = pkt_readMessageHeader(&data);
	struct SerializeHeader serial = pkt_readSerializeHeader(&data);
	if(len <= 414) {
		if(reliable)
			master_net_send_reliable(ctx, session, buf, len, pkt_readBaseMasterServerReliableRequest(&data).requestId, 1, message.type != MessageType_HandshakeMessage);
		else
			net_send_internal(ctx, &session->net, buf, len, message.type != MessageType_HandshakeMessage);
		return;
	}
	if(message.type == MessageType_UserMessage) {
		fprintf(stderr, "[MASTER] serialize UserMessageType_UserMultipartMessage (%s)\n", reflect(UserMessageType, serial.type));
		serial.type = UserMessageType_UserMultipartMessage;
	} else if(message.type == MessageType_DedicatedServerMessage) {
		fprintf(stderr, "[MASTER] serialize DedicatedServerMessageType_DedicatedServerMultipartMessage (%s)\n", reflect(DedicatedServerMessageType, serial.type));
		serial.type = DedicatedServerMessageType_DedicatedServerMultipartMessage;
	} else if(message.type == MessageType_HandshakeMessage) {
		fprintf(stderr, "[MASTER] serialize HandshakeMessageType_HandshakeMultipartMessage (%s)\n", reflect(HandshakeMessageType, serial.type));
		serial.type = HandshakeMessageType_HandshakeMultipartMessage;
	} else {
		return;
	}
	pkt_readNetPacketHeader(&buf);
	len = end - buf;

	struct BaseMasterServerMultipartMessage mp;
	mp.multipartMessageId = pkt_readBaseMasterServerReliableRequest((const uint8_t*[]){data}).requestId;
	mp.offset = 0;
	mp.length = 384;
	mp.totalLength = len;
	do {
		mp.base.requestId = master_getNextRequestId(session);
		if(len - mp.offset < mp.length)
			mp.length = len - mp.offset;
		uint8_t mpbuf[512];
		uint8_t *mpbuf_end = mpbuf;
		memcpy(mp.data, &buf[mp.offset], mp.length);
		pkt_writeNetPacketHeader(&mpbuf_end, (struct NetPacketHeader){
			.property = PacketProperty_UnconnectedMessage,
			.connectionNumber = 0,
			.isFragmented = 0,
		});
		pkt_writeMessageHeader(&mpbuf_end, message);
		uint8_t *msg_end = mpbuf_end;
		pkt_writeBaseMasterServerMultipartMessage(&msg_end, mp);
		serial.length = msg_end + 1 - mpbuf_end;
		pkt_writeSerializeHeader(&mpbuf_end, serial);
		pkt_writeBaseMasterServerMultipartMessage(&mpbuf_end, mp);
		master_net_send_reliable(ctx, session, mpbuf, mpbuf_end - mpbuf, mp.base.requestId, 1, message.type != MessageType_HandshakeMessage);
		mp.offset += 384;
	} while(mp.offset < len);
	master_net_send_reliable(ctx, session, buf, data - buf, mp.multipartMessageId, 0, message.type != MessageType_HandshakeMessage);
}

static void master_send_ack(struct Context *ctx, struct MasterSession *session, MessageType type, uint32_t requestId) {
	struct BaseMasterServerAcknowledgeMessage r_ack;
	r_ack.base.responseId = requestId;
	r_ack.messageHandled = 1;
	uint8_t resp[65536], *resp_end = resp;
	if(type == MessageType_UserMessage)
		MASTER_SERIALIZE(&resp_end, UserMessage, UserMessageReceivedAcknowledge, BaseMasterServerAcknowledgeMessage, r_ack)
	else if(type == MessageType_DedicatedServerMessage)
		MASTER_SERIALIZE(&resp_end, DedicatedServerMessage, DedicatedServerMessageReceivedAcknowledge, BaseMasterServerAcknowledgeMessage, r_ack)
	else if(type == MessageType_HandshakeMessage)
		MASTER_SERIALIZE(&resp_end, HandshakeMessage, HandshakeMessageReceivedAcknowledge, BaseMasterServerAcknowledgeMessage, r_ack)
	master_send(&ctx->net, session, resp, resp_end - resp, 0);
}

static void handle_ClientHelloRequest(struct Context *ctx, struct MasterSession *session, const uint8_t **data) {
	if(session->handshakeStep != 255) {
		if(net_time() - NetSession_get_lastKeepAlive(&session->net) < 5000) // 5 second timeout to prevent clients from getting "locked out" if their previous session hasn't closed or timed out yet
			return;
		net_session_reset(&ctx->net, &session->net); // security or something idk
		session->resend.count = 0;
	}
	struct ClientHelloRequest req = pkt_readClientHelloRequest(data);
	session->epoch = req.base.requestId & 0xff000000;
	memcpy(session->net.clientRandom, req.random, 32);
	struct HelloVerifyRequest r_hello;
	r_hello.base.requestId = 0;
	r_hello.base.responseId = req.base.requestId;
	memcpy(r_hello.cookie, NetSession_get_cookie(&session->net), sizeof(r_hello.cookie));
	uint8_t resp[65536], *resp_end = resp;
	MASTER_SERIALIZE(&resp_end, HandshakeMessage, HelloVerifyRequest, HelloVerifyRequest, r_hello);
	master_send(&ctx->net, session, resp, resp_end - resp, 0);
	session->handshakeStep = HandshakeMessageType_ClientHelloRequest;
}

static void handle_ClientHelloWithCookieRequest(struct Context *ctx, struct MasterSession *session, const uint8_t **data) {
	struct ClientHelloWithCookieRequest req = pkt_readClientHelloWithCookieRequest(data);
	master_send_ack(ctx, session, MessageType_HandshakeMessage, req.base.requestId);
	if(session->handshakeStep != HandshakeMessageType_ClientHelloRequest)
		return;
	if(memcmp(req.cookie, NetSession_get_cookie(&session->net), 32) != 0)
		return;
	if(memcmp(req.random, session->net.clientRandom, 32) != 0)
		return;
	session->ClientHelloWithCookieRequest_requestId = req.base.requestId; // don't @ me   -rc

	struct ServerCertificateRequest r_cert;
	r_cert.base.requestId = master_getNextRequestId(session);
	r_cert.base.responseId = req.certificateResponseId;
	r_cert.certificateCount = 0;
	for(mbedtls_x509_crt *it = ctx->cert; it; it = it->MBEDTLS_PRIVATE(next)) {
		r_cert.certificateList[r_cert.certificateCount].length = it->MBEDTLS_PRIVATE(raw).MBEDTLS_PRIVATE(len);
		memcpy(r_cert.certificateList[r_cert.certificateCount].data, it->MBEDTLS_PRIVATE(raw).MBEDTLS_PRIVATE(p), r_cert.certificateList[r_cert.certificateCount].length);
		++r_cert.certificateCount;
	}
	uint8_t resp[65536], *resp_end = resp;
	MASTER_SERIALIZE(&resp_end, HandshakeMessage, ServerCertificateRequest, ServerCertificateRequest, r_cert);
	master_send(&ctx->net, session, resp, resp_end - resp, 1);
	session->handshakeStep = HandshakeMessageType_ClientHelloWithCookieRequest;
}

static void handle_ServerCertificateRequest_ack(struct Context *ctx, struct MasterSession *session) {
	if(session->handshakeStep != HandshakeMessageType_ClientHelloWithCookieRequest)
		return;
	struct ServerHelloRequest r_hello;
	r_hello.base.requestId = master_getNextRequestId(session);
	r_hello.base.responseId = session->ClientHelloWithCookieRequest_requestId;
	memcpy(r_hello.random, NetSession_get_serverRandom(&session->net), sizeof(r_hello.random));
	r_hello.publicKey.length = sizeof(r_hello.publicKey.data);
	if(NetSession_write_key(&session->net, &ctx->net, r_hello.publicKey.data, &r_hello.publicKey.length))
		return;
	{
		uint8_t sig[r_hello.publicKey.length + 64];
		memcpy(sig, session->net.clientRandom, 32);
		memcpy(&sig[32], NetSession_get_serverRandom(&session->net), 32);
		memcpy(&sig[64], r_hello.publicKey.data, r_hello.publicKey.length);
		NetSession_signature(&session->net, &ctx->net, ctx->key, sig, sizeof(sig), &r_hello.signature);
	}

	uint8_t resp[65536], *resp_end = resp;
	MASTER_SERIALIZE(&resp_end, HandshakeMessage, ServerHelloRequest, ServerHelloRequest, r_hello);
	master_send(&ctx->net, session, resp, resp_end - resp, 1);
	session->handshakeStep = HandshakeMessageType_ServerCertificateRequest;
}

static void handle_ClientKeyExchangeRequest(struct Context *ctx, struct MasterSession *session, const uint8_t **data) {
	struct ClientKeyExchangeRequest req = pkt_readClientKeyExchangeRequest(data);
	master_send_ack(ctx, session, MessageType_HandshakeMessage, req.base.requestId);
	if(session->handshakeStep != HandshakeMessageType_ServerCertificateRequest)
		return;
	if(NetSession_set_clientPublicKey(&session->net, &ctx->net, &req.clientPublicKey))
		return;
	struct ChangeCipherSpecRequest r_spec;
	r_spec.base.requestId = master_getNextRequestId(session);
	r_spec.base.responseId = req.base.requestId;
	uint8_t resp[65536], *resp_end = resp;
	MASTER_SERIALIZE(&resp_end, HandshakeMessage, ChangeCipherSpecRequest, ChangeCipherSpecRequest, r_spec);
	master_send(&ctx->net, session, resp, resp_end - resp, 1);
	session->handshakeStep = HandshakeMessageType_ClientKeyExchangeRequest;
}

static void handle_AuthenticateUserRequest(struct Context *ctx, struct MasterSession *session, const uint8_t **data) {
	struct AuthenticateUserRequest req = pkt_readAuthenticateUserRequest(data);
	master_send_ack(ctx, session, MessageType_UserMessage, req.base.requestId);
	struct AuthenticateUserResponse r_auth;
	r_auth.base.requestId = master_getNextRequestId(session);
	r_auth.base.responseId = req.base.requestId;
	r_auth.result = AuthenticateUserResponse_Result_Success;
	uint8_t resp[65536], *resp_end = resp;
	MASTER_SERIALIZE(&resp_end, UserMessage, AuthenticateUserResponse, AuthenticateUserResponse, r_auth);
	master_send(&ctx->net, session, resp, resp_end - resp, 1);
}

static void handle_ConnectToServerRequest(struct Context *ctx, struct MasterSession *session, const uint8_t **data) {
	struct ConnectToServerRequest req = pkt_readConnectToServerRequest(data);
	master_send_ack(ctx, session, MessageType_UserMessage, req.base.base.requestId);
	// TODO: deduplicate this request
	struct ConnectToServerResponse r_conn;
	r_conn.base.requestId = master_getNextRequestId(session);
	r_conn.base.responseId = req.base.base.requestId;
	if(req.code == StringToServerCode(NULL, 0)) {
		if(req.selectionMask.difficulties != BeatmapDifficultyMask_All && req.selectionMask.modifiers == GameplayModifierMask_NoFail && req.configuration.maxPlayerCount == 5 && req.configuration.discoveryPolicy == DiscoveryPolicy_Public && req.configuration.invitePolicy == InvitePolicy_AnyoneCanInvite && req.configuration.gameplayServerMode == GameplayServerMode_Countdown && req.configuration.songSelectionMode == SongSelectionMode_Vote && req.configuration.gameplayServerControlSettings == GameplayServerControlSettings_None) {
			r_conn.result = ConnectToServerResponse_Result_NoAvailableDedicatedServers; // Quick Play not yet available
			goto send;
		}
		if(instance_open(&req.code)) {
			r_conn.result = ConnectToServerResponse_Result_NoAvailableDedicatedServers;
			goto send;
		}
	} else {
		if(!instance_get_isopen(req.code)) {
			r_conn.result = ConnectToServerResponse_Result_InvalidCode;
			goto send;
		}
	}
	{
		struct SS addr = *NetSession_get_addr(&session->net);
		struct NetContext *net = instance_get_net(req.code);
		struct NetSession *isession = instance_resolve_session(req.code, addr);
		if(!isession) {
			r_conn.result = ConnectToServerResponse_Result_UnknownError;
			goto send;
		}
		memcpy(isession->clientRandom, req.base.random, 32);
		memcpy(r_conn.random, NetSession_get_serverRandom(isession), 32);
		r_conn.publicKey.length = sizeof(r_conn.publicKey.data);
		if(NetSession_write_key(isession, net, r_conn.publicKey.data, &r_conn.publicKey.length)) {
			r_conn.result = ConnectToServerResponse_Result_UnknownError;
			goto send;
		}
		if(NetSession_set_clientPublicKey(isession, net, &req.base.publicKey)) {
			r_conn.result = ConnectToServerResponse_Result_UnknownError;
			goto send;
		}
		r_conn.userId.length = sprintf(r_conn.userId.data, "dtxJlHm56k6ZXcnxhbyfiA");
		r_conn.userName.length = 0;
		r_conn.secret = req.secret;
		r_conn.selectionMask = req.selectionMask;
		r_conn.flags = 3;
		r_conn.remoteEndPoint = instance_get_address(req.code);
		r_conn.code = req.code;
		r_conn.configuration = req.configuration;
		r_conn.managerId = req.base.userId; // ?
		r_conn.result = ConnectToServerResponse_Result_Success;
	}
	send:;
	uint8_t resp[65536], *resp_end = resp;
	MASTER_SERIALIZE(&resp_end, UserMessage, ConnectToServerResponse, ConnectToServerResponse, r_conn);
	master_send(&ctx->net, session, resp, resp_end - resp, 1);
}

#ifdef WINDOWS
static DWORD WINAPI
#else
static void*
#endif
master_handler(struct Context *ctx) {
	fprintf(stderr, "[MASTER] Started\n");
	uint8_t buf[262144];
	memset(buf, 0, sizeof(buf));
	uint32_t len;
	struct MasterSession *session;
	const uint8_t *pkt;
	while((len = net_recv(&ctx->net, buf, sizeof(buf), (struct NetSession**)&session, &pkt))) {
		const uint8_t *data = pkt, *end = &pkt[len];
		struct NetPacketHeader header = pkt_readNetPacketHeader(&data);
		if(header.property != PacketProperty_UnconnectedMessage) {
			fprintf(stderr, "[MASTER] Unsupported packet type: %s\n", reflect(PacketProperty, header.property));
			continue;
		}
		struct MessageHeader message = pkt_readMessageHeader(&data);
		struct SerializeHeader serial = pkt_readSerializeHeader(&data);
		debug_logType(message, serial);
		if(message.type == MessageType_UserMessage) {
			switch(serial.type) {
				case UserMessageType_AuthenticateUserRequest: handle_AuthenticateUserRequest(ctx, session, &data); break;
				case UserMessageType_AuthenticateUserResponse: fprintf(stderr, "[MASTER] BAD TYPE: UserMessageType_AuthenticateUserResponse\n"); break;
				case UserMessageType_ConnectToServerResponse: fprintf(stderr, "[MASTER] BAD TYPE: UserMessageType_ConnectToServerResponse\n"); break;
				case UserMessageType_ConnectToServerRequest: handle_ConnectToServerRequest(ctx, session, &data); break;
				case UserMessageType_UserMessageReceivedAcknowledge: {
					master_handle_ack(session, &message, &serial, pkt_readBaseMasterServerAcknowledgeMessage(&data).base.responseId);
					break;
				}
				case UserMessageType_UserMultipartMessage: fprintf(stderr, "[MASTER] UserMessageType_UserMultipartMessage not implemented\n"); return 0;
				case UserMessageType_SessionKeepaliveMessage: break;
				case UserMessageType_GetPublicServersRequest: fprintf(stderr, "[MASTER] UserMessageType_GetPublicServersRequest not implemented\n"); return 0;
				case UserMessageType_GetPublicServersResponse: fprintf(stderr, "[MASTER] UserMessageType_GetPublicServersResponse not implemented\n"); return 0;
				default: fprintf(stderr, "[MASTER] BAD USER MESSAGE TYPE\n");
			}
		} else if(message.type == MessageType_DedicatedServerMessage) {
			fprintf(stderr, "[MASTER] DedicatedServerMessageType not implemented\n");
		} else if(message.type == MessageType_HandshakeMessage) {
			switch(serial.type) {
				case HandshakeMessageType_ClientHelloRequest: handle_ClientHelloRequest(ctx, session, &data); break;
				case HandshakeMessageType_HelloVerifyRequest: fprintf(stderr, "[MASTER] BAD TYPE: HandshakeMessageType_HelloVerifyRequest\n"); break;
				case HandshakeMessageType_ClientHelloWithCookieRequest: handle_ClientHelloWithCookieRequest(ctx, session, &data); break;
				case HandshakeMessageType_ServerHelloRequest: fprintf(stderr, "[MASTER] BAD TYPE: HandshakeMessageType_ServerHelloRequest\n"); break;
				case HandshakeMessageType_ServerCertificateRequest: fprintf(stderr, "[MASTER] BAD TYPE: HandshakeMessageType_ServerCertificateRequest\n"); break;
				case HandshakeMessageType_ClientKeyExchangeRequest: handle_ClientKeyExchangeRequest(ctx, session, &data); break;
				case HandshakeMessageType_ChangeCipherSpecRequest: fprintf(stderr, "[MASTER] BAD TYPE: HandshakeMessageType_ChangeCipherSpecRequest\n"); break;
				case HandshakeMessageType_HandshakeMessageReceivedAcknowledge: {
					if(master_handle_ack(session, &message, &serial, pkt_readBaseMasterServerAcknowledgeMessage(&data).base.responseId)) {
						if(message.type == MessageType_HandshakeMessage && serial.type == HandshakeMessageType_ServerCertificateRequest)
							handle_ServerCertificateRequest_ack(ctx, session);
					}
					break;
				}
				case HandshakeMessageType_HandshakeMultipartMessage: fprintf(stderr, "[MASTER] HandshakeMessageType_HandshakeMultipartMessage not implemented\n"); return 0;
				default: fprintf(stderr, "[MASTER] BAD HANDSHAKE MESSAGE TYPE\n");
			}
		} else {
			fprintf(stderr, "[MASTER] BAD MESSAGE TYPE\n");
		}
		if(data != end)
			fprintf(stderr, "[MASTER] BAD PACKET LENGTH (expected %u, got %zu)\n", len, data - pkt);
	}
	return 0;
}

#ifdef WINDOWS
static HANDLE master_thread = NULL;
#else
static pthread_t master_thread = 0;
#endif
static struct Context ctx = {NULL, NULL, NULL, {-1}};
_Bool master_init(mbedtls_x509_crt *cert, mbedtls_pk_context *key, uint16_t port) {
	{
		uint_fast8_t count = 0;
		for(mbedtls_x509_crt *it = cert; it; it = it->MBEDTLS_PRIVATE(next), ++count) {
			if(it->MBEDTLS_PRIVATE(raw).MBEDTLS_PRIVATE(len) > 4096) {
				fprintf(stderr, "[MASTER] Host certificate too large\n");
				return 1;
			}
		}
		if(count > lengthof(((struct ServerCertificateRequest*)NULL)->certificateList)) {
			fprintf(stderr, "[MASTER] Host certificate chain too long\n");
			return 1;
		}
	}
	ctx.cert = cert;
	ctx.key = key;
	if(net_init(&ctx.net, port)) {
		fprintf(stderr, "[MASTER] net_init() failed\n");
		return 1;
	}
	ctx.net.user = &ctx;
	ctx.net.onResolve = master_onResolve;
	ctx.net.onResend = master_onResend;
	#ifdef WINDOWS
	master_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)master_handler, &ctx, 0, NULL);
	return !master_thread;
	#else
	return pthread_create(&master_thread, NULL, (void*(*)(void*))&master_handler, &ctx) != 0;
	#endif
}

void master_cleanup() {
	if(master_thread) {
		net_stop(&ctx.net);
		fprintf(stderr, "[MASTER] Stopping\n");
		#ifdef WINDOWS
		WaitForSingleObject(master_thread, INFINITE);
		#else
		pthread_join(master_thread, NULL);
		#endif
		master_thread = 0;
	}
	while(ctx.sessionList)
		ctx.sessionList = master_disconnect(ctx.sessionList);
	net_cleanup(&ctx.net);
}
