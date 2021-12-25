#include "packets.h"
#include <stdio.h>

void debug_logMessage(struct MessageHeader message, struct SerializeHeader serial) {
	fprintf(stderr, "\tmessage.type=%s\n", reflect(MessageType, message.type));
	fprintf(stderr, "\tmessage.protocolVersion=%u\n", message.protocolVersion);
	fprintf(stderr, "\tserial.length=%u\n", serial.length);
	if(message.type == MessageType_UserMessage)
		fprintf(stderr, "\tserial.type=%s\n", reflect(UserMessageType, serial.type));
	else if(message.type == MessageType_DedicatedServerMessage)
		fprintf(stderr, "\tserial.type=%s\n", reflect(DedicatedServerMessageType, serial.type));
	else if(message.type == MessageType_HandshakeMessage)
		fprintf(stderr, "\tserial.type=%s\n", reflect(HandshakeMessageType, serial.type));
}

void debug_logType(struct MessageHeader message, struct SerializeHeader serial) {
	if(message.type == MessageType_UserMessage)
		fprintf(stderr, "recieve UserMessageType_%s\n", reflect(UserMessageType, serial.type));
	else if(message.type == MessageType_DedicatedServerMessage)
		fprintf(stderr, "recieve DedicatedServerMessageType_%s\n", reflect(DedicatedServerMessageType, serial.type));
	else if(message.type == MessageType_HandshakeMessage)
		fprintf(stderr, "recieve HandshakeMessageType_%s\n", reflect(HandshakeMessageType, serial.type));
}

void debug_logPacket(const uint8_t *data) {
	struct MessageHeader message = pkt_readMessageHeader(&data);
	struct SerializeHeader serial = pkt_readSerializeHeader(&data);
	debug_logMessage(message, serial);
	char buf[1024];
	if(message.type == MessageType_UserMessage) {
		switch(serial.type) {
			case UserMessageType_AuthenticateUserRequest: pkt_logAuthenticateUserRequest("\tAuthenticateUserRequest", buf, buf, pkt_readAuthenticateUserRequest(&data)); break;
			case UserMessageType_AuthenticateUserResponse: pkt_logAuthenticateUserResponse("\tAuthenticateUserResponse", buf, buf, pkt_readAuthenticateUserResponse(&data)); break;
			case UserMessageType_ConnectToServerResponse: pkt_logConnectToServerResponse("\tConnectToServerResponse", buf, buf, pkt_readConnectToServerResponse(&data)); break;
			case UserMessageType_ConnectToServerRequest: pkt_logConnectToServerRequest("\tConnectToServerRequest", buf, buf, pkt_readConnectToServerRequest(&data)); break;
			case UserMessageType_UserMessageReceivedAcknowledge: pkt_logUserMessageReceivedAcknowledge("\tUserMessageReceivedAcknowledge", buf, buf, pkt_readUserMessageReceivedAcknowledge(&data)); break;
			case UserMessageType_UserMultipartMessage: pkt_logUserMultipartMessage("\tUserMultipartMessage", buf, buf, pkt_readUserMultipartMessage(&data)); break;
			case UserMessageType_SessionKeepaliveMessage: pkt_logSessionKeepaliveMessage("\tSessionKeepaliveMessage", buf, buf, pkt_readSessionKeepaliveMessage(&data)); break;
			case UserMessageType_GetPublicServersRequest: pkt_logGetPublicServersRequest("\tGetPublicServersRequest", buf, buf, pkt_readGetPublicServersRequest(&data)); break;
			case UserMessageType_GetPublicServersResponse: pkt_logGetPublicServersResponse("\tGetPublicServersResponse", buf, buf, pkt_readGetPublicServersResponse(&data)); break;
		}
	} else if(message.type == MessageType_DedicatedServerMessage) {
		switch(serial.type) {
			case DedicatedServerMessageType_AuthenticateDedicatedServerRequest: pkt_logAuthenticateDedicatedServerRequest("\tAuthenticateDedicatedServerRequest", buf, buf, pkt_readAuthenticateDedicatedServerRequest(&data)); break;
			case DedicatedServerMessageType_AuthenticateDedicatedServerResponse: pkt_logAuthenticateDedicatedServerResponse("\tAuthenticateDedicatedServerResponse", buf, buf, pkt_readAuthenticateDedicatedServerResponse(&data)); break;
			case DedicatedServerMessageType_CreateDedicatedServerInstanceRequest: pkt_logCreateDedicatedServerInstanceRequest("\tCreateDedicatedServerInstanceRequest", buf, buf, pkt_readCreateDedicatedServerInstanceRequest(&data)); break;
			case DedicatedServerMessageType_CreateDedicatedServerInstanceResponse: pkt_logCreateDedicatedServerInstanceResponse("\tCreateDedicatedServerInstanceResponse", buf, buf, pkt_readCreateDedicatedServerInstanceResponse(&data)); break;
			case DedicatedServerMessageType_DedicatedServerInstanceNoLongerAvailableRequest: pkt_logDedicatedServerInstanceNoLongerAvailableRequest("\tDedicatedServerInstanceNoLongerAvailableRequest", buf, buf, pkt_readDedicatedServerInstanceNoLongerAvailableRequest(&data)); break;
			case DedicatedServerMessageType_DedicatedServerHeartbeatRequest: pkt_logDedicatedServerHeartbeatRequest("\tDedicatedServerHeartbeatRequest", buf, buf, pkt_readDedicatedServerHeartbeatRequest(&data)); break;
			case DedicatedServerMessageType_DedicatedServerHeartbeatResponse: pkt_logDedicatedServerHeartbeatResponse("\tDedicatedServerHeartbeatResponse", buf, buf, pkt_readDedicatedServerHeartbeatResponse(&data)); break;
			case DedicatedServerMessageType_DedicatedServerInstanceStatusUpdateRequest: pkt_logDedicatedServerInstanceStatusUpdateRequest("\tDedicatedServerInstanceStatusUpdateRequest", buf, buf, pkt_readDedicatedServerInstanceStatusUpdateRequest(&data)); break;
			case DedicatedServerMessageType_DedicatedServerShutDownRequest: pkt_logDedicatedServerShutDownRequest("\tDedicatedServerShutDownRequest", buf, buf, pkt_readDedicatedServerShutDownRequest(&data)); break;
			case DedicatedServerMessageType_DedicatedServerPrepareForConnectionRequest: pkt_logDedicatedServerPrepareForConnectionRequest("\tDedicatedServerPrepareForConnectionRequest", buf, buf, pkt_readDedicatedServerPrepareForConnectionRequest(&data)); break;
			case DedicatedServerMessageType_DedicatedServerMessageReceivedAcknowledge: pkt_logDedicatedServerMessageReceivedAcknowledge("\tDedicatedServerMessageReceivedAcknowledge", buf, buf, pkt_readDedicatedServerMessageReceivedAcknowledge(&data)); break;
			case DedicatedServerMessageType_DedicatedServerMultipartMessage: pkt_logDedicatedServerMultipartMessage("\tDedicatedServerMultipartMessage", buf, buf, pkt_readDedicatedServerMultipartMessage(&data)); break;
			case DedicatedServerMessageType_DedicatedServerPrepareForConnectionResponse: pkt_logDedicatedServerPrepareForConnectionResponse("\tDedicatedServerPrepareForConnectionResponse", buf, buf, pkt_readDedicatedServerPrepareForConnectionResponse(&data)); break;
		}
	} else if(message.type == MessageType_HandshakeMessage) {
		switch(serial.type) {
			case HandshakeMessageType_ClientHelloRequest: pkt_logClientHelloRequest("\tClientHelloRequest", buf, buf, pkt_readClientHelloRequest(&data)); break;
			case HandshakeMessageType_HelloVerifyRequest: pkt_logHelloVerifyRequest("\tHelloVerifyRequest", buf, buf, pkt_readHelloVerifyRequest(&data)); break;
			case HandshakeMessageType_ClientHelloWithCookieRequest: pkt_logClientHelloWithCookieRequest("\tClientHelloWithCookieRequest", buf, buf, pkt_readClientHelloWithCookieRequest(&data)); break;
			case HandshakeMessageType_ServerHelloRequest: pkt_logServerHelloRequest("\tServerHelloRequest", buf, buf, pkt_readServerHelloRequest(&data)); break;
			case HandshakeMessageType_ServerCertificateRequest: pkt_logServerCertificateRequest("\tServerCertificateRequest", buf, buf, pkt_readServerCertificateRequest(&data)); break;
			case HandshakeMessageType_ServerCertificateResponse: pkt_logServerCertificateResponse("\tServerCertificateResponse", buf, buf, pkt_readServerCertificateResponse(&data)); break;
			case HandshakeMessageType_ClientKeyExchangeRequest: pkt_logClientKeyExchangeRequest("\tClientKeyExchangeRequest", buf, buf, pkt_readClientKeyExchangeRequest(&data)); break;
			case HandshakeMessageType_ChangeCipherSpecRequest: pkt_logChangeCipherSpecRequest("\tChangeCipherSpecRequest", buf, buf, pkt_readChangeCipherSpecRequest(&data)); break;
			case HandshakeMessageType_HandshakeMessageReceivedAcknowledge: pkt_logHandshakeMessageReceivedAcknowledge("\tHandshakeMessageReceivedAcknowledge", buf, buf, pkt_readHandshakeMessageReceivedAcknowledge(&data)); break;
			case HandshakeMessageType_HandshakeMultipartMessage: pkt_logHandshakeMultipartMessage("\tHandshakeMultipartMessage", buf, buf, pkt_readHandshakeMultipartMessage(&data)); break;
		}
	} else {
		fprintf(stderr, "BAD TYPE\n");
	}
}