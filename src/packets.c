/* 
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 */

#include "packets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t pkt_readUint8(uint8_t **pkt) {
	uint8_t v = (*pkt)[0];
	*pkt += sizeof(v);
	return v;
}
static uint16_t pkt_readUint16(uint8_t **pkt) {
	uint16_t v = (*pkt)[0] | (*pkt)[1] << 8;
	*pkt += sizeof(v);
	return v;
}
static uint32_t pkt_readUint32(uint8_t **pkt) {
	uint32_t v = (*pkt)[0] | (*pkt)[1] << 8 | (*pkt)[2] << 16 | (*pkt)[3] << 24;
	*pkt += sizeof(v);
	return v;
}
static uint64_t pkt_readUint64(uint8_t **pkt) {
	uint64_t v = (uint64_t)(*pkt)[0] | (uint64_t)(*pkt)[1] << 8 | (uint64_t)(*pkt)[2] << 16 | (uint64_t)(*pkt)[3] << 24 | (uint64_t)(*pkt)[4] << 32 | (uint64_t)(*pkt)[5] << 40 | (uint64_t)(*pkt)[6] << 48 | (uint64_t)(*pkt)[7] << 56;
	*pkt += sizeof(v);
	return v;
}
static uint64_t pkt_readVarUint64(uint8_t **pkt) {
	uint64_t byte, value = 0;
	uint8_t shift = 0;
	do {
		byte = pkt_readUint8(pkt);
		value |= (byte & 127) << shift;
		shift += 7;
	} while(byte & 128);
	return value;
}
static int64_t pkt_readVarInt64(uint8_t **pkt) {
	int64_t varULong = (int64_t)pkt_readVarUint64(pkt);
	if((varULong & 1L) != 1L)
		return varULong >> 1;
	return -(varULong >> 1) + 1L;
}
static uint32_t pkt_readVarUint32(uint8_t **pkt) {
	return (uint32_t)pkt_readVarUint64(pkt);
}
static int32_t pkt_readVarInt32(uint8_t **pkt) {
	return (int32_t)pkt_readVarInt64(pkt);
}
static void pkt_readBytes(uint8_t **pkt, uint8_t *out, uint32_t count) {
	memcpy(out, *pkt, count);
	*pkt += count;
}
static void pkt_writeUint8(uint8_t **pkt, uint8_t v) {
	(*pkt)[0] = v;
	*pkt += sizeof(v);
}
static void pkt_writeUint16(uint8_t **pkt, uint16_t v) {
	(*pkt)[0] = v & 255;
	(*pkt)[1] = v >> 8 & 255;
	*pkt += sizeof(v);
}
static void pkt_writeUint32(uint8_t **pkt, uint32_t v) {
	(*pkt)[0] = v & 255;
	(*pkt)[1] = v >> 8 & 255;
	(*pkt)[2] = v >> 16 & 255;
	(*pkt)[3] = v >> 24 & 255;
	*pkt += sizeof(v);
}
static void pkt_writeUint64(uint8_t **pkt, uint64_t v) {
	(*pkt)[0] = v & 255;
	(*pkt)[1] = v >> 8 & 255;
	(*pkt)[2] = v >> 16 & 255;
	(*pkt)[3] = v >> 24 & 255;
	(*pkt)[4] = v >> 32 & 255;
	(*pkt)[5] = v >> 40 & 255;
	(*pkt)[6] = v >> 48 & 255;
	(*pkt)[7] = v >> 56 & 255;
	*pkt += sizeof(v);
}
static void pkt_writeVarUint64(uint8_t **pkt, uint64_t v) {
	do {
		uint8_t byte = v & 127;
		v >>= 7;
		if(v)
			byte |= 128;
		pkt_writeUint8(pkt, byte);
	} while(v);
}
static void pkt_writeVarUint32(uint8_t **pkt, uint32_t v) {
	pkt_writeVarUint64(pkt, v);
}
void pkt_writeBytes(uint8_t **pkt, uint8_t *in, uint32_t count) {
	memcpy(*pkt, in, count);
	*pkt += count;
}
struct PacketEncryptionLayer pkt_readPacketEncryptionLayer(uint8_t **pkt) {
	struct PacketEncryptionLayer out;
	out.encrypted = pkt_readUint8(pkt);
	if(out.encrypted) {
		fprintf(stderr, "READ ENCRYPTED PACKET\n");
		abort();
	}
	return out;
}
void pkt_writePacketEncryptionLayer(uint8_t **pkt, struct PacketEncryptionLayer in) {
	pkt_writeUint8(pkt, in.encrypted);
	if(in.encrypted) {
		fprintf(stderr, "WRITE ENCRYPTED PACKET\n");
		abort();
	}
}
static const uint8_t NetPacketHeaderSize[32] = {
	1, 4, 4, 3, 11, 14, 11, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};
struct NetPacketHeader pkt_readNetPacketHeader(uint8_t **pkt) {
	struct NetPacketHeader out;
	uint8_t bits = pkt_readUint8(pkt);
	out.property = bits & 31;
	out.connectionNumber = bits >> 5 & 3;
	out.isFragmented = bits >> 7 & 1;
	out.sequence = (NetPacketHeaderSize[out.property] < 3) ? 0 : pkt_readUint16(pkt);
	out.channelId = (NetPacketHeaderSize[out.property] < 4) ? 0 : pkt_readUint8(pkt);
	out.fragmentId = (NetPacketHeaderSize[out.property] < 6) ? 0 : pkt_readUint16(pkt);
	out.fragmentPart = (NetPacketHeaderSize[out.property] < 8) ? 0 : pkt_readUint16(pkt);
	out.fragmentsTotal = (NetPacketHeaderSize[out.property] < 10) ? 0 : pkt_readUint16(pkt);
	return out;
}
void pkt_writeNetPacketHeader(uint8_t **pkt, struct NetPacketHeader in) {
	pkt_writeUint8(pkt, in.property | in.connectionNumber << 5 | in.isFragmented << 7);
	if(NetPacketHeaderSize[in.property] >= 3) pkt_writeUint16(pkt, in.sequence);
	if(NetPacketHeaderSize[in.property] >= 4) pkt_writeUint8(pkt, in.channelId);
	if(NetPacketHeaderSize[in.property] >= 6) pkt_writeUint16(pkt, in.fragmentId);
	if(NetPacketHeaderSize[in.property] >= 8) pkt_writeUint16(pkt, in.fragmentPart);
	if(NetPacketHeaderSize[in.property] >= 10) pkt_writeUint16(pkt, in.fragmentsTotal);
}
struct MessageHeader pkt_readMessageHeader(uint8_t **pkt) {
	struct MessageHeader out;
	out.type = pkt_readUint32(pkt);
	out.protocolVersion = pkt_readVarUint32(pkt);
	return out;
}
struct SerializeHeader pkt_readSerializeHeader(uint8_t **pkt) {
	struct SerializeHeader out;
	out.length = pkt_readVarUint32(pkt);
	out.type = pkt_readUint8(pkt);
	return out;
}
struct BaseMasterServerReliableRequest pkt_readBaseMasterServerReliableRequest(uint8_t **pkt) {
	struct BaseMasterServerReliableRequest out;
	out.requestId = pkt_readUint32(pkt);
	return out;
}
struct BaseMasterServerResponse pkt_readBaseMasterServerResponse(uint8_t **pkt) {
	struct BaseMasterServerResponse out;
	out.responseId = pkt_readUint32(pkt);
	return out;
}
struct BaseMasterServerReliableResponse pkt_readBaseMasterServerReliableResponse(uint8_t **pkt) {
	struct BaseMasterServerReliableResponse out;
	out.requestId = pkt_readUint32(pkt);
	out.responseId = pkt_readUint32(pkt);
	return out;
}
struct BaseMasterServerAcknowledgeMessage pkt_readBaseMasterServerAcknowledgeMessage(uint8_t **pkt) {
	struct BaseMasterServerAcknowledgeMessage out;
	out.base = pkt_readBaseMasterServerResponse(pkt);
	out.messageHandled = pkt_readUint8(pkt);
	return out;
}
struct ByteArrayNetSerializable pkt_readByteArrayNetSerializable(uint8_t **pkt) {
	struct ByteArrayNetSerializable out;
	out.length = pkt_readVarUint32(pkt);
	pkt_readBytes(pkt, out.data, out.length);
	return out;
}
struct String pkt_readString(uint8_t **pkt) {
	struct String out;
	out.length = pkt_readUint32(pkt);
	pkt_readBytes(pkt, out.data, out.length);
	return out;
}
struct AuthenticationToken pkt_readAuthenticationToken(uint8_t **pkt) {
	struct AuthenticationToken out;
	out.platform = pkt_readUint8(pkt);
	out.userId = pkt_readString(pkt);
	out.userName = pkt_readString(pkt);
	out.sessionToken = pkt_readByteArrayNetSerializable(pkt);
	return out;
}
struct AuthenticateUserRequest pkt_readAuthenticateUserRequest(uint8_t **pkt) {
	struct AuthenticateUserRequest out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	out.authenticationToken = pkt_readAuthenticationToken(pkt);
	return out;
}
struct ConnectToServerResponse pkt_readConnectToServerResponse(uint8_t **pkt) {
	struct ConnectToServerResponse out;
	return out;
}
struct ConnectToServerRequest pkt_readConnectToServerRequest(uint8_t **pkt) {
	struct ConnectToServerRequest out;
	return out;
}
struct UserMessageReceivedAcknowledge pkt_readUserMessageReceivedAcknowledge(uint8_t **pkt) {
	struct UserMessageReceivedAcknowledge out;
	return out;
}
struct SessionKeepaliveMessage pkt_readSessionKeepaliveMessage(uint8_t **pkt) {
	struct SessionKeepaliveMessage out;
	return out;
}
struct GetPublicServersRequest pkt_readGetPublicServersRequest(uint8_t **pkt) {
	struct GetPublicServersRequest out;
	return out;
}
struct GetPublicServersResponse pkt_readGetPublicServersResponse(uint8_t **pkt) {
	struct GetPublicServersResponse out;
	return out;
}
struct AuthenticateDedicatedServerRequest pkt_readAuthenticateDedicatedServerRequest(uint8_t **pkt) {
	struct AuthenticateDedicatedServerRequest out;
	return out;
}
struct AuthenticateDedicatedServerResponse pkt_readAuthenticateDedicatedServerResponse(uint8_t **pkt) {
	struct AuthenticateDedicatedServerResponse out;
	return out;
}
struct CreateDedicatedServerInstanceRequest pkt_readCreateDedicatedServerInstanceRequest(uint8_t **pkt) {
	struct CreateDedicatedServerInstanceRequest out;
	return out;
}
struct CreateDedicatedServerInstanceResponse pkt_readCreateDedicatedServerInstanceResponse(uint8_t **pkt) {
	struct CreateDedicatedServerInstanceResponse out;
	return out;
}
struct DedicatedServerInstanceNoLongerAvailableRequest pkt_readDedicatedServerInstanceNoLongerAvailableRequest(uint8_t **pkt) {
	struct DedicatedServerInstanceNoLongerAvailableRequest out;
	return out;
}
struct DedicatedServerHeartbeatRequest pkt_readDedicatedServerHeartbeatRequest(uint8_t **pkt) {
	struct DedicatedServerHeartbeatRequest out;
	return out;
}
struct DedicatedServerHeartbeatResponse pkt_readDedicatedServerHeartbeatResponse(uint8_t **pkt) {
	struct DedicatedServerHeartbeatResponse out;
	return out;
}
struct DedicatedServerInstanceStatusUpdateRequest pkt_readDedicatedServerInstanceStatusUpdateRequest(uint8_t **pkt) {
	struct DedicatedServerInstanceStatusUpdateRequest out;
	return out;
}
struct DedicatedServerShutDownRequest pkt_readDedicatedServerShutDownRequest(uint8_t **pkt) {
	struct DedicatedServerShutDownRequest out;
	return out;
}
struct DedicatedServerPrepareForConnectionRequest pkt_readDedicatedServerPrepareForConnectionRequest(uint8_t **pkt) {
	struct DedicatedServerPrepareForConnectionRequest out;
	return out;
}
struct DedicatedServerMessageReceivedAcknowledge pkt_readDedicatedServerMessageReceivedAcknowledge(uint8_t **pkt) {
	struct DedicatedServerMessageReceivedAcknowledge out;
	return out;
}
struct DedicatedServerPrepareForConnectionResponse pkt_readDedicatedServerPrepareForConnectionResponse(uint8_t **pkt) {
	struct DedicatedServerPrepareForConnectionResponse out;
	return out;
}
struct ClientHelloRequest pkt_readClientHelloRequest(uint8_t **pkt) {
	struct ClientHelloRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(pkt);
	pkt_readBytes(pkt, out.random, 32);
	return out;
}
struct ClientHelloWithCookieRequest pkt_readClientHelloWithCookieRequest(uint8_t **pkt) {
	struct ClientHelloWithCookieRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(pkt);
	out.certificateResponseId = pkt_readUint32(pkt);
	pkt_readBytes(pkt, out.random, 32);
	pkt_readBytes(pkt, out.cookie, 32);
	return out;
}
struct ServerCertificateResponse pkt_readServerCertificateResponse(uint8_t **pkt) {
	struct ServerCertificateResponse out;
	return out;
}
struct ClientKeyExchangeRequest pkt_readClientKeyExchangeRequest(uint8_t **pkt) {
	struct ClientKeyExchangeRequest out;
	return out;
}
struct ChangeCipherSpecRequest pkt_readChangeCipherSpecRequest(uint8_t **pkt) {
	struct ChangeCipherSpecRequest out;
	return out;
}
struct HandshakeMessageReceivedAcknowledge pkt_readHandshakeMessageReceivedAcknowledge(uint8_t **pkt) {
	struct HandshakeMessageReceivedAcknowledge out;
	return out;
}
void pkt_writeMessageHeader(uint8_t **pkt, struct MessageHeader in) {
	pkt_writeUint32(pkt, in.type);
	pkt_writeVarUint32(pkt, in.protocolVersion);
}
void pkt_writeSerializeHeader(uint8_t **pkt, struct SerializeHeader in) {
	pkt_writeVarUint32(pkt, in.length);
	pkt_writeUint8(pkt, in.type);
}
void pkt_writeBaseMasterServerReliableRequest(uint8_t **pkt, struct BaseMasterServerReliableRequest in) {
	pkt_writeUint32(pkt, in.requestId);
}
void pkt_writeBaseMasterServerResponse(uint8_t **pkt, struct BaseMasterServerResponse in) {
	pkt_writeUint32(pkt, in.responseId);
}
void pkt_writeBaseMasterServerReliableResponse(uint8_t **pkt, struct BaseMasterServerReliableResponse in) {
	pkt_writeUint32(pkt, in.requestId);
	pkt_writeUint32(pkt, in.responseId);
}
void pkt_writeBaseMasterServerAcknowledgeMessage(uint8_t **pkt, struct BaseMasterServerAcknowledgeMessage in) {
	pkt_writeBaseMasterServerResponse(pkt, in.base);
	pkt_writeUint8(pkt, in.messageHandled);
}
void pkt_writeByteArrayNetSerializable(uint8_t **pkt, struct ByteArrayNetSerializable in) {
	pkt_writeVarUint32(pkt, in.length);
	pkt_writeBytes(pkt, in.data, in.length);
}
void pkt_writeBaseMasterServerMultipartMessage(uint8_t **pkt, struct BaseMasterServerMultipartMessage in) {
	pkt_writeBaseMasterServerReliableRequest(pkt, in.base);
	pkt_writeUint32(pkt, in.multipartMessageId);
	pkt_writeVarUint32(pkt, in.offset);
	pkt_writeVarUint32(pkt, in.length);
	pkt_writeVarUint32(pkt, in.totalLength);
	pkt_writeBytes(pkt, in.data, in.length);
}
void pkt_writeAuthenticateUserResponse(uint8_t **pkt, struct AuthenticateUserResponse in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeUint8(pkt, in.result);
}
void pkt_writeConnectToServerResponse(uint8_t **pkt, struct ConnectToServerResponse in) {
}
void pkt_writeConnectToServerRequest(uint8_t **pkt, struct ConnectToServerRequest in) {
}
void pkt_writeUserMessageReceivedAcknowledge(uint8_t **pkt, struct UserMessageReceivedAcknowledge in) {
}
void pkt_writeUserMultipartMessage(uint8_t **pkt, struct UserMultipartMessage in) {
	pkt_writeBaseMasterServerMultipartMessage(pkt, in.base);
}
void pkt_writeSessionKeepaliveMessage(uint8_t **pkt, struct SessionKeepaliveMessage in) {
}
void pkt_writeGetPublicServersRequest(uint8_t **pkt, struct GetPublicServersRequest in) {
}
void pkt_writeGetPublicServersResponse(uint8_t **pkt, struct GetPublicServersResponse in) {
}
void pkt_writeAuthenticateDedicatedServerRequest(uint8_t **pkt, struct AuthenticateDedicatedServerRequest in) {
}
void pkt_writeAuthenticateDedicatedServerResponse(uint8_t **pkt, struct AuthenticateDedicatedServerResponse in) {
}
void pkt_writeCreateDedicatedServerInstanceRequest(uint8_t **pkt, struct CreateDedicatedServerInstanceRequest in) {
}
void pkt_writeCreateDedicatedServerInstanceResponse(uint8_t **pkt, struct CreateDedicatedServerInstanceResponse in) {
}
void pkt_writeDedicatedServerInstanceNoLongerAvailableRequest(uint8_t **pkt, struct DedicatedServerInstanceNoLongerAvailableRequest in) {
}
void pkt_writeDedicatedServerHeartbeatRequest(uint8_t **pkt, struct DedicatedServerHeartbeatRequest in) {
}
void pkt_writeDedicatedServerHeartbeatResponse(uint8_t **pkt, struct DedicatedServerHeartbeatResponse in) {
}
void pkt_writeDedicatedServerInstanceStatusUpdateRequest(uint8_t **pkt, struct DedicatedServerInstanceStatusUpdateRequest in) {
}
void pkt_writeDedicatedServerShutDownRequest(uint8_t **pkt, struct DedicatedServerShutDownRequest in) {
}
void pkt_writeDedicatedServerPrepareForConnectionRequest(uint8_t **pkt, struct DedicatedServerPrepareForConnectionRequest in) {
}
void pkt_writeDedicatedServerMessageReceivedAcknowledge(uint8_t **pkt, struct DedicatedServerMessageReceivedAcknowledge in) {
}
void pkt_writeDedicatedServerMultipartMessage(uint8_t **pkt, struct DedicatedServerMultipartMessage in) {
	pkt_writeBaseMasterServerMultipartMessage(pkt, in.base);
}
void pkt_writeDedicatedServerPrepareForConnectionResponse(uint8_t **pkt, struct DedicatedServerPrepareForConnectionResponse in) {
}
void pkt_writeHelloVerifyRequest(uint8_t **pkt, struct HelloVerifyRequest in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeBytes(pkt, in.cookie, 32);
}
void pkt_writeServerHelloRequest(uint8_t **pkt, struct ServerHelloRequest in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeBytes(pkt, in.random, 32);
	pkt_writeByteArrayNetSerializable(pkt, in.publicKey);
	pkt_writeByteArrayNetSerializable(pkt, in.signature);
}
void pkt_writeServerCertificateRequest(uint8_t **pkt, struct ServerCertificateRequest in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeVarUint32(pkt, in.certificateCount);
	for(uint32_t i = 0; i < in.certificateCount; ++i)
		pkt_writeByteArrayNetSerializable(pkt, in.certificateList[i]);
}
void pkt_writeClientKeyExchangeRequest(uint8_t **pkt, struct ClientKeyExchangeRequest in) {
}
void pkt_writeChangeCipherSpecRequest(uint8_t **pkt, struct ChangeCipherSpecRequest in) {
}
void pkt_writeHandshakeMessageReceivedAcknowledge(uint8_t **pkt, struct HandshakeMessageReceivedAcknowledge in) {
}
void pkt_writeHandshakeMultipartMessage(uint8_t **pkt, struct HandshakeMultipartMessage in) {
	pkt_writeBaseMasterServerMultipartMessage(pkt, in.base);
}
