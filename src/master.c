#include "master.h"
#include "enum_reflection.h"
#include "net.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>

_Bool master_run() {
	uint32_t len;
	struct MasterServerSession *session;
	PacketProperty property;
	uint8_t *pkt;
	while((len = net_recv(&session, &property, &pkt)) > 0) {
		uint8_t *data = pkt, *end = &pkt[len];
		if(property == PacketProperty_UnconnectedMessage) {
			struct MessageHeader message = pkt_readMessageHeader(&data);
			struct SerializeHeader serial = pkt_readSerializeHeader(&data);
			#if 0
			debug_logMessage(message, serial);
			#else
			debug_logType(message, serial);
			#endif
			if(message.type == MessageType_UserMessage) {
				switch(serial.type) {
					case UserMessageType_AuthenticateUserRequest: fprintf(stderr, "UserMessageType_AuthenticateUserRequest not implemented\n"); return 1;
					case UserMessageType_AuthenticateUserResponse: fprintf(stderr, "UserMessageType_AuthenticateUserResponse not implemented\n"); return 1;
					case UserMessageType_ConnectToServerResponse: fprintf(stderr, "UserMessageType_ConnectToServerResponse not implemented\n"); return 1;
					case UserMessageType_ConnectToServerRequest: fprintf(stderr, "UserMessageType_ConnectToServerRequest not implemented\n"); return 1;
					case UserMessageType_MessageReceivedAcknowledge: fprintf(stderr, "UserMessageType_MessageReceivedAcknowledge not implemented\n"); return 1;
					case UserMessageType_MultipartMessage: fprintf(stderr, "UserMessageType_MultipartMessage not implemented\n"); return 1;
					case UserMessageType_SessionKeepaliveMessage: fprintf(stderr, "UserMessageType_SessionKeepaliveMessage not implemented\n"); return 1;
					case UserMessageType_GetPublicServersRequest: fprintf(stderr, "UserMessageType_GetPublicServersRequest not implemented\n"); return 1;
					case UserMessageType_GetPublicServersResponse: fprintf(stderr, "UserMessageType_GetPublicServersResponse not implemented\n"); return 1;
					default: fprintf(stderr, "BAD TYPE\n");
				}
			} else if(message.type == MessageType_DedicatedServerMessage) {
				switch(serial.type) {
					case DedicatedServerMessageType_AuthenticateDedicatedServerRequest: fprintf(stderr, "DedicatedServerMessageType_AuthenticateDedicatedServerRequest not implemented\n"); return 1;
					case DedicatedServerMessageType_AuthenticateDedicatedServerResponse: fprintf(stderr, "DedicatedServerMessageType_AuthenticateDedicatedServerResponse not implemented\n"); return 1;
					case DedicatedServerMessageType_CreateDedicatedServerInstanceRequest: fprintf(stderr, "DedicatedServerMessageType_CreateDedicatedServerInstanceRequest not implemented\n"); return 1;
					case DedicatedServerMessageType_CreateDedicatedServerInstanceResponse: fprintf(stderr, "DedicatedServerMessageType_CreateDedicatedServerInstanceResponse not implemented\n"); return 1;
					case DedicatedServerMessageType_DedicatedServerInstanceNoLongerAvailableRequest: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerInstanceNoLongerAvailableRequest not implemented\n"); return 1;
					case DedicatedServerMessageType_DedicatedServerHeartbeatRequest: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerHeartbeatRequest not implemented\n"); return 1;
					case DedicatedServerMessageType_DedicatedServerHeartbeatResponse: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerHeartbeatResponse not implemented\n"); return 1;
					case DedicatedServerMessageType_DedicatedServerInstanceStatusUpdateRequest: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerInstanceStatusUpdateRequest not implemented\n"); return 1;
					case DedicatedServerMessageType_DedicatedServerShutDownRequest: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerShutDownRequest not implemented\n"); return 1;
					case DedicatedServerMessageType_DedicatedServerPrepareForConnectionRequest: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerPrepareForConnectionRequest not implemented\n"); return 1;
					case DedicatedServerMessageType_MessageReceivedAcknowledge: fprintf(stderr, "DedicatedServerMessageType_MessageReceivedAcknowledge not implemented\n"); return 1;
					case DedicatedServerMessageType_MultipartMessage: fprintf(stderr, "DedicatedServerMessageType_MultipartMessage not implemented\n"); return 1;
					case DedicatedServerMessageType_DedicatedServerPrepareForConnectionResponse: fprintf(stderr, "DedicatedServerMessageType_DedicatedServerPrepareForConnectionResponse not implemented\n"); return 1;
					default: fprintf(stderr, "BAD TYPE\n");
				}
			} else if(message.type == MessageType_HandshakeMessage) {
				switch(serial.type) {
					case HandshakeMessageType_ClientHelloRequest: {
						struct ClientHelloRequest req = pkt_readClientHelloRequest(&data);
						session->epoch = req.base.requestId & 0xff000000;
						session->state = MasterServerSessionState_New;
						memcpy(session->clientRandom, req.random, sizeof(session->clientRandom));
						struct HelloVerifyRequest r_hello;
						r_hello.base.requestId = 0; // net_getNextRequestId(session);
						r_hello.base.responseId = req.base.requestId;
						memcpy(r_hello.cookie, session->cookie, sizeof(r_hello.cookie));
						uint8_t *resp = pkt;
						#if 0
						SERIALIZE(&resp, message, HandshakeMessage, HelloVerifyRequest, HelloVerifyRequest, r_hello);
						#else
						SERIALIZE_HEAD(&resp, message, HandshakeMessage);
						SERIALIZE_BODY(&resp, HandshakeMessageType_HelloVerifyRequest, HelloVerifyRequest, r_hello);
						#endif
						net_send(session, PacketProperty_UnconnectedMessage, pkt, resp - pkt);
						break;
					}
					case HandshakeMessageType_HelloVerifyRequest: fprintf(stderr, "BAD TYPE: HandshakeMessageType_HelloVerifyRequest\n"); break;
					case HandshakeMessageType_ClientHelloWithCookieRequest: {
						struct ClientHelloWithCookieRequest req = pkt_readClientHelloWithCookieRequest(&data);
						if(memcmp(req.cookie, session->cookie, sizeof(session->cookie)) != 0)
							break;
						if(memcmp(req.random, session->clientRandom, sizeof(session->clientRandom)) != 0)
							break;
						struct HandshakeMessageReceivedAcknowledge r_ack;
						r_ack.base.base.responseId = req.base.requestId;
						r_ack.base.messageHandled = 1;
						uint8_t *resp = pkt;
						SERIALIZE(&resp, message, HandshakeMessage, MessageReceivedAcknowledge, HandshakeMessageReceivedAcknowledge, r_ack);
						net_send(session, PacketProperty_UnconnectedMessage, pkt, resp - pkt);

						struct ServerCertificateRequest r_cert;
						r_cert.base.requestId = 1; // net_getNextRequestId(session);
						r_cert.base.responseId = req.certificateResponseId;
						r_cert.certificateCount = 0;
						// r_cert.certificateList[0] = cert;
						resp = pkt;
						SERIALIZE(&resp, message, HandshakeMessage, ServerCertificateRequest, ServerCertificateRequest, r_cert);
						net_send(session, PacketProperty_UnconnectedMessage, pkt, resp - pkt);

						struct ServerHelloRequest r_hello;
						r_hello.base.requestId = net_getNextRequestId(session);
						r_hello.base.responseId = req.base.requestId;
						memcpy(r_hello.random, session->serverRandom, sizeof(r_hello.random));
						size_t keylen = 0;
						if(mbedtls_ecp_point_write_binary(&session->key.grp, &session->key.Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &keylen, r_hello.publicKey.data, 256) != 0) {
							fprintf(stderr, "mbedtls_ecp_point_write_binary() failed\n");
							break;
						}
						r_hello.publicKey.length = keylen;
						r_hello.signature.length = 0;
						memcpy(r_hello.signature.data, session->clientRandom, sizeof(session->clientRandom));
						r_hello.signature.length += sizeof(session->clientRandom);
						memcpy(&r_hello.signature.data[r_hello.signature.length], session->serverRandom, sizeof(session->serverRandom));
						r_hello.signature.length += sizeof(session->serverRandom);
						memcpy(&r_hello.signature.data[r_hello.signature.length], r_hello.publicKey.data, r_hello.publicKey.length);
						r_hello.signature.length += r_hello.publicKey.length;
						resp = pkt;
						SERIALIZE(&resp, message, HandshakeMessage, ServerHelloRequest, ServerHelloRequest, r_hello);
						net_send(session, PacketProperty_UnconnectedMessage, pkt, resp - pkt);
						break;
					}
					case HandshakeMessageType_ServerHelloRequest: fprintf(stderr, "BAD TYPE: HandshakeMessageType_ServerHelloRequest\n"); break;
					case HandshakeMessageType_ServerCertificateRequest: fprintf(stderr, "BAD TYPE: HandshakeMessageType_ServerCertificateRequest\n"); break;
					case HandshakeMessageType_ServerCertificateResponse: fprintf(stderr, "HandshakeMessageType_ServerCertificateResponse not implemented\n"); return 1;
					case HandshakeMessageType_ClientKeyExchangeRequest:
						fprintf(stderr, "HandshakeMessageType_ClientKeyExchangeRequest not implemented\n");
						// ACTIVATE ENCRYPTION HERE
						// send HandshakeMessageType_ChangeCipherSpecRequest
						break;
					case HandshakeMessageType_ChangeCipherSpecRequest: fprintf(stderr, "BAD TYPE: HandshakeMessageType_ChangeCipherSpecRequest\n"); break;
					case HandshakeMessageType_MessageReceivedAcknowledge: data = end; break; // ignore
					case HandshakeMessageType_MultipartMessage: fprintf(stderr, "HandshakeMessageType_MultipartMessage not implemented\n"); return 1;
					default: fprintf(stderr, "BAD TYPE\n");
				}
			} else {
				fprintf(stderr, "BAD TYPE\n");
			}
		} else {
			fprintf(stderr, "Unsupported packet type: %s\n", reflect(PacketProperty, property));
		}
		if(data != end)
			fprintf(stderr, "BAD PACKET LENGTH (expected %u, got %zu)\n", len, data - pkt);
	}
	return 0;
}