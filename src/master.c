#include "enum_reflection.h"
#include "net.h"
#ifdef WINDOWS
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif
#include "debug.h"
#include <mbedtls/entropy.h>
#include <stdio.h>
#include <string.h>

#define lengthof(x) (sizeof(x)/sizeof(*x))

struct Context {
	mbedtls_x509_crt *cert;
	int32_t sockfd;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
};

static void send_ack(struct Context *ctx, struct MasterServerSession *session, uint8_t *pkt, struct MessageHeader message, struct BaseMasterServerReliableRequest req) {
	struct BaseMasterServerAcknowledgeMessage r_ack;
	r_ack.base.responseId = req.requestId;
	r_ack.messageHandled = 1;
	uint8_t *resp = pkt;
	if(message.type == MessageType_UserMessage)
		SERIALIZE(&resp, message, UserMessage, UserMessageReceivedAcknowledge, BaseMasterServerAcknowledgeMessage, r_ack)
	else if(message.type == MessageType_DedicatedServerMessage)
		SERIALIZE(&resp, message, DedicatedServerMessage, DedicatedServerMessageReceivedAcknowledge, BaseMasterServerAcknowledgeMessage, r_ack)
	else if(message.type == MessageType_HandshakeMessage)
		SERIALIZE(&resp, message, HandshakeMessage, HandshakeMessageReceivedAcknowledge, BaseMasterServerAcknowledgeMessage, r_ack)
	net_send(ctx->sockfd, session, PacketType_UnconnectedMessage, pkt, resp - pkt, 0);
}

#ifdef WINDOWS
static DWORD WINAPI
#else
static void*
#endif
master_handler(struct Context *ctx) {
	fprintf(stderr, "Master server started\n");
	uint32_t len;
	struct MasterServerSession *session;
	PacketType property;
	uint8_t *pkt;
	while((len = net_recv(ctx->sockfd, &ctx->ctr_drbg, &session, &property, &pkt))) {
		uint8_t *data = pkt, *end = &pkt[len];
		if(property == PacketType_UnconnectedMessage) {
			struct MessageHeader message = pkt_readMessageHeader(&data);
			struct SerializeHeader serial = pkt_readSerializeHeader(&data);
			#if 0
			debug_logMessage(message, serial);
			#else
			debug_logType(message, serial);
			#endif
			if(message.type == MessageType_UserMessage) {
				switch(serial.type) {
					case UserMessageType_AuthenticateUserRequest: fprintf(stderr, "UserMessageType_AuthenticateUserRequest not implemented\n"); return 0;
					case UserMessageType_AuthenticateUserResponse: fprintf(stderr, "UserMessageType_AuthenticateUserResponse not implemented\n"); return 0;
					case UserMessageType_ConnectToServerResponse: fprintf(stderr, "UserMessageType_ConnectToServerResponse not implemented\n"); return 0;
					case UserMessageType_ConnectToServerRequest: fprintf(stderr, "UserMessageType_ConnectToServerRequest not implemented\n"); return 0;
					case UserMessageType_UserMessageReceivedAcknowledge: fprintf(stderr, "UserMessageType_UserMessageReceivedAcknowledge not implemented\n"); return 0;
					case UserMessageType_UserMultipartMessage: fprintf(stderr, "UserMessageType_UserMultipartMessage not implemented\n"); return 0;
					case UserMessageType_SessionKeepaliveMessage: fprintf(stderr, "UserMessageType_SessionKeepaliveMessage not implemented\n"); return 0;
					case UserMessageType_GetPublicServersRequest: fprintf(stderr, "UserMessageType_GetPublicServersRequest not implemented\n"); return 0;
					case UserMessageType_GetPublicServersResponse: fprintf(stderr, "UserMessageType_GetPublicServersResponse not implemented\n"); return 0;
					default: fprintf(stderr, "BAD TYPE\n");
				}
			} else if(message.type == MessageType_DedicatedServerMessage) {
				switch(serial.type) {
					case DedicatedServerMessageType_AuthenticateDedicatedServerRequest: fprintf(stderr, "DedicatedServerMessageType_AuthenticateDedicatedServerRequest not implemented\n"); return 0;
					case DedicatedServerMessageType_AuthenticateDedicatedServerResponse: fprintf(stderr, "DedicatedServerMessageType_AuthenticateDedicatedServerResponse not implemented\n"); return 0;
					case DedicatedServerMessageType_CreateDedicatedServerInstanceRequest: fprintf(stderr, "DedicatedServerMessageType_CreateDedicatedServerInstanceRequest not implemented\n"); return 0;
					case DedicatedServerMessageType_CreateDedicatedServerInstanceResponse: fprintf(stderr, "DedicatedServerMessageType_CreateDedicatedServerInstanceResponse not implemented\n"); return 0;
					case DedicatedServerMessageType_DedicatedServerInstanceNoLongerAvailableRequest: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerInstanceNoLongerAvailableRequest not implemented\n"); return 0;
					case DedicatedServerMessageType_DedicatedServerHeartbeatRequest: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerHeartbeatRequest not implemented\n"); return 0;
					case DedicatedServerMessageType_DedicatedServerHeartbeatResponse: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerHeartbeatResponse not implemented\n"); return 0;
					case DedicatedServerMessageType_DedicatedServerInstanceStatusUpdateRequest: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerInstanceStatusUpdateRequest not implemented\n"); return 0;
					case DedicatedServerMessageType_DedicatedServerShutDownRequest: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerShutDownRequest not implemented\n"); return 0;
					case DedicatedServerMessageType_DedicatedServerPrepareForConnectionRequest: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerPrepareForConnectionRequest not implemented\n"); return 0;
					case DedicatedServerMessageType_DedicatedServerMessageReceivedAcknowledge: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerMessageReceivedAcknowledge not implemented\n"); return 0;
					case DedicatedServerMessageType_DedicatedServerMultipartMessage: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerMultipartMessage not implemented\n"); return 0;
					case DedicatedServerMessageType_DedicatedServerPrepareForConnectionResponse: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerPrepareForConnectionResponse not implemented\n"); return 0;
					default: fprintf(stderr, "BAD TYPE\n");
				}
			} else if(message.type == MessageType_HandshakeMessage) {
				switch(serial.type) {
					case HandshakeMessageType_ClientHelloRequest: {
						struct ClientHelloRequest req = pkt_readClientHelloRequest(&data);
						MasterServerSession_set_epoch(session, req.base.requestId & 0xff000000);
						MasterServerSession_set_state(session, MasterServerSessionState_New);
						memcpy(MasterServerSession_get_clientRandom(session), req.random, 32);
						struct HelloVerifyRequest r_hello;
						r_hello.base.requestId = 0; // net_getNextRequestId(session);
						r_hello.base.responseId = req.base.requestId;
						memcpy(r_hello.cookie, MasterServerSession_get_cookie(session), sizeof(r_hello.cookie));
						uint8_t *resp = pkt;
						#if 0
						SERIALIZE(&resp, message, HandshakeMessage, HelloVerifyRequest, HelloVerifyRequest, r_hello);
						#else
						SERIALIZE_HEAD(&resp, message, HandshakeMessage);
						SERIALIZE_BODY(&resp, HandshakeMessageType_HelloVerifyRequest, HelloVerifyRequest, r_hello);
						#endif
						net_send(ctx->sockfd, session, PacketType_UnconnectedMessage, pkt, resp - pkt, 0);
						break;
					}
					case HandshakeMessageType_HelloVerifyRequest: fprintf(stderr, "BAD TYPE: HandshakeMessageType_HelloVerifyRequest\n"); break;
					case HandshakeMessageType_ClientHelloWithCookieRequest: {
						struct ClientHelloWithCookieRequest req = pkt_readClientHelloWithCookieRequest(&data);
						send_ack(ctx, session, pkt, message, req.base);
						if(memcmp(req.cookie, MasterServerSession_get_cookie(session), 32) != 0)
							break;
						if(memcmp(req.random, MasterServerSession_get_clientRandom(session), 32) != 0)
							break;

						struct ServerCertificateRequest r_cert;
						r_cert.base.requestId = 1; // net_getNextRequestId(session);
						r_cert.base.responseId = req.certificateResponseId;
						r_cert.certificateCount = 0;
						for(mbedtls_x509_crt *it = ctx->cert; it; it = it->MBEDTLS_PRIVATE(next)) {
							r_cert.certificateList[r_cert.certificateCount].length = it->MBEDTLS_PRIVATE(raw).MBEDTLS_PRIVATE(len);
							memcpy(r_cert.certificateList[r_cert.certificateCount].data, it->MBEDTLS_PRIVATE(raw).MBEDTLS_PRIVATE(p), r_cert.certificateList[r_cert.certificateCount].length);
							++r_cert.certificateCount;
						}
						uint8_t *resp = pkt;
						SERIALIZE(&resp, message, HandshakeMessage, ServerCertificateRequest, ServerCertificateRequest, r_cert);
						net_send(ctx->sockfd, session, PacketType_UnconnectedMessage, pkt, resp - pkt, 1);

						struct ServerHelloRequest r_hello;
						r_hello.base.requestId = net_getNextRequestId(session);
						r_hello.base.responseId = req.base.requestId;
						memcpy(r_hello.random, MasterServerSession_get_serverRandom(session), sizeof(r_hello.random));
						if(MasterServerSession_write_key(session, &r_hello.publicKey))
							break;
						r_hello.signature.length = 0;
						memcpy(r_hello.signature.data, MasterServerSession_get_clientRandom(session), 32);
						r_hello.signature.length += 32;
						memcpy(&r_hello.signature.data[r_hello.signature.length], MasterServerSession_get_serverRandom(session), 32);
						r_hello.signature.length += 32;
						memcpy(&r_hello.signature.data[r_hello.signature.length], r_hello.publicKey.data, r_hello.publicKey.length);
						r_hello.signature.length += r_hello.publicKey.length;
						resp = pkt;
						SERIALIZE(&resp, message, HandshakeMessage, ServerHelloRequest, ServerHelloRequest, r_hello);
						net_send(ctx->sockfd, session, PacketType_UnconnectedMessage, pkt, resp - pkt, 1);
						break;
					}
					case HandshakeMessageType_ServerHelloRequest: fprintf(stderr, "BAD TYPE: HandshakeMessageType_ServerHelloRequest\n"); break;
					case HandshakeMessageType_ServerCertificateRequest: fprintf(stderr, "BAD TYPE: HandshakeMessageType_ServerCertificateRequest\n"); break;
					case HandshakeMessageType_ServerCertificateResponse: fprintf(stderr, "HandshakeMessageType_ServerCertificateResponse not implemented\n"); return 0;
					case HandshakeMessageType_ClientKeyExchangeRequest:
						fprintf(stderr, "HandshakeMessageType_ClientKeyExchangeRequest not implemented\n");
						// ACTIVATE ENCRYPTION HERE
						// send HandshakeMessageType_ChangeCipherSpecRequest
						break;
					case HandshakeMessageType_ChangeCipherSpecRequest: fprintf(stderr, "BAD TYPE: HandshakeMessageType_ChangeCipherSpecRequest\n"); break;
					case HandshakeMessageType_HandshakeMessageReceivedAcknowledge: {
						struct BaseMasterServerAcknowledgeMessage ack = pkt_readBaseMasterServerAcknowledgeMessage(&data);
						uint8_t *resp = net_handle_ack(session, ack.base.responseId);
						if(!resp) // expected in prodution; only error when testing on local network
							fprintf(stderr, "UNMATCHED ACK: %u\n", ack.base.responseId);
						break;
					}
					case HandshakeMessageType_HandshakeMultipartMessage: fprintf(stderr, "HandshakeMessageType_HandshakeMultipartMessage not implemented\n"); return 0;
					default: fprintf(stderr, "BAD TYPE\n");
				}
			} else {
				fprintf(stderr, "BAD TYPE\n");
			}
		} else {
			fprintf(stderr, "Unsupported packet type: %s\n", reflect(PacketType, property));
		}
		if(data != end)
			fprintf(stderr, "BAD PACKET LENGTH (expected %u, got %zu)\n", len, data - pkt);
	}
	return 0;
}

#ifdef WINDOWS
static HANDLE master_thread;
#else
static pthread_t master_thread;
#endif
static struct Context ctx = {NULL, -1};
_Bool master_init(mbedtls_x509_crt *cert, uint16_t port) {
	{
		uint_fast8_t count = 0;
		for(mbedtls_x509_crt *it = cert; it; it = it->MBEDTLS_PRIVATE(next), ++count) {
			if(it->MBEDTLS_PRIVATE(raw).MBEDTLS_PRIVATE(len) > 4096) {
				fprintf(stderr, "Host certificate too large\n");
				return 1;
			}
		}
		if(count > lengthof(((struct ServerCertificateRequest*)NULL)->certificateList)) {
			fprintf(stderr, "Host certificate chain too long\n");
			return 1;
		}
	}
	ctx.cert = cert;
	ctx.sockfd = net_init(port);
	if(ctx.sockfd == -1) {
		fprintf(stderr, "net_init() failed\n");
		return 1;
	}
	mbedtls_ctr_drbg_init(&ctx.ctr_drbg);
	mbedtls_entropy_init(&ctx.entropy);
	if(mbedtls_ctr_drbg_seed(&ctx.ctr_drbg, mbedtls_entropy_func, &ctx.entropy, (const uint8_t*)u8"M@$73RS€RV3R", 14) != 0) {
		fprintf(stderr, "mbedtls_ctr_drbg_seed() failed\n");
		return 1;
	}
	#ifdef WINDOWS
	master_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)master_handler, &ctx, 0, NULL);
	return !master_thread;
	#else
	return pthread_create(&master_thread, NULL, (void*)&master_handler, &ctx) != 0;
	#endif
}

void master_cleanup() {
	if(ctx.sockfd != -1) {
		fprintf(stderr, "Stopping master server\n");
		mbedtls_entropy_free(&ctx.entropy);
		mbedtls_ctr_drbg_free(&ctx.ctr_drbg);
		net_cleanup(ctx.sockfd);
		ctx.sockfd = -1;
		#ifdef WINDOWS
		WaitForSingleObject(master_thread, INFINITE);
		#else
		pthread_join(master_thread, NULL);
		#endif
	}
}
