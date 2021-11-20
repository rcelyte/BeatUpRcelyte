#include "packets.h"
#include "serial.h"
#include <stdio.h>

/*
DANGEROUS CLASSES (contain ByteArrayNetSerializable):
	ClientHelloWithCookieRequest
	ServerCertificateRequest
	AuthenticateDedicatedServerRequest
	CreateDedicatedServerInstanceResponse
	DedicatedServerPrepareForConnectionRequest
	HelloVerifyRequest
	BaseConnectToServerRequest
	ServerHelloRequest
	ClientKeyExchangeRequest
	CreateDedicatedServerInstanceRequest
	ConnectToServerResponse
*/

struct PacketEncryptionLayer pkt_readPacketEncryptionLayer(uint8_t **pkt) {
	struct PacketEncryptionLayer out;
	out.encrypted = pkt_readUint8(pkt);
	if(out.encrypted) {
		fprintf(stderr, "READ ENCRYPTED PACKET\n");
		abort();
	}
	return out;
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
/*struct ByteArrayNetSerializable pkt_readByteArrayNetSerializable(uint8_t **pkt) {
	struct ByteArrayNetSerializable out;
	out.length = pkt_readVarUint32(pkt);
	if(out.length > sizeof(out.data))
		out.length = 0;
	pkt_readBytesTo(pkt, out.data, out.length);
	return out;
}*/
/*struct String pkt_readString(uint8_t **pkt) {
	struct String out;
	out.length = pkt_readUint32(pkt);
	pkt_readBytesTo(pkt, out.data, out.length);
	return out;
}*/
struct BaseMasterServerReliableRequest pkt_readBaseMasterServerReliableRequest(uint8_t **pkt) {
	struct BaseMasterServerReliableRequest out;
	out.requestId = pkt_readUint32(pkt);
	return out;
}
static void pkt_writeBaseMasterServerReliableRequest(uint8_t **pkt, struct BaseMasterServerReliableRequest in) {
	pkt_writeUint32(pkt, in.requestId);
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
void pkt_writePacketEncryptionLayer(uint8_t **pkt, struct PacketEncryptionLayer in) {
	pkt_writeUint8(pkt, in.encrypted);
	if(in.encrypted) {
		fprintf(stderr, "WRITE ENCRYPTED PACKET\n");
		abort();
	}
}
void pkt_writeNetPacketHeader(uint8_t **pkt, struct NetPacketHeader in) {
	pkt_writeUint8(pkt, in.property | in.connectionNumber << 5 | in.isFragmented << 7);
	if(NetPacketHeaderSize[in.property] >= 3) pkt_writeUint16(pkt, in.sequence);
	if(NetPacketHeaderSize[in.property] >= 4) pkt_writeUint8(pkt, in.channelId);
	if(NetPacketHeaderSize[in.property] >= 6) pkt_writeUint16(pkt, in.fragmentId);
	if(NetPacketHeaderSize[in.property] >= 8) pkt_writeUint16(pkt, in.fragmentPart);
	if(NetPacketHeaderSize[in.property] >= 10) pkt_writeUint16(pkt, in.fragmentsTotal);
}
void pkt_writeMessageHeader(uint8_t **pkt, struct MessageHeader in) {
	pkt_writeUint32(pkt, in.type);
	pkt_writeVarUint32(pkt, in.protocolVersion);
}
void pkt_writeSerializeHeader(uint8_t **pkt, struct SerializeHeader in) {
	pkt_writeVarUint32(pkt, in.length);
	pkt_writeUint8(pkt, in.type);
}
static void pkt_writeByteArrayNetSerializable(uint8_t **pkt, struct ByteArrayNetSerializable in) {
	pkt_writeVarUint32(pkt, in.length);
	pkt_writeBytes(pkt, in.data, in.length);
}
static void pkt_writeBaseMasterServerReliableResponse(uint8_t **pkt, struct BaseMasterServerReliableResponse in) {
	pkt_writeUint32(pkt, in.requestId);
	pkt_writeUint32(pkt, in.responseId);
}
void pkt_writeHelloVerifyRequest(uint8_t **pkt, struct HelloVerifyRequest in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeBytes(pkt, in.cookie, 32);
}
void pkt_writeServerCertificateRequest(uint8_t **pkt, struct ServerCertificateRequest in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeVarUint32(pkt, in.certificateCount);
	for(uint32_t i = 0; i < in.certificateCount; ++i)
		pkt_writeByteArrayNetSerializable(pkt, in.certificateList[i]);
}
void pkt_writeServerHelloRequest(uint8_t **pkt, struct ServerHelloRequest in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeBytes(pkt, in.random, 32);
	pkt_writeByteArrayNetSerializable(pkt, in.publicKey);
	pkt_writeByteArrayNetSerializable(pkt, in.signature);
}
void pkt_writeBaseMasterServerResponse(uint8_t **pkt, struct BaseMasterServerResponse in) {
	pkt_writeUint32(pkt, in.responseId);
}
void pkt_writeBaseMasterServerAcknowledgeMessage(uint8_t **pkt, struct BaseMasterServerAcknowledgeMessage in) {
	pkt_writeBaseMasterServerResponse(pkt, in.base);
	pkt_writeUint8(pkt, in.messageHandled);
}
void pkt_writeHandshakeMessageReceivedAcknowledge(uint8_t **pkt, struct HandshakeMessageReceivedAcknowledge in) {
	pkt_writeBaseMasterServerAcknowledgeMessage(pkt, in.base);
}
void pkt_writeDedicatedServerMessageReceivedAcknowledge(uint8_t **pkt, struct DedicatedServerMessageReceivedAcknowledge in) {
	pkt_writeBaseMasterServerAcknowledgeMessage(pkt, in.base);
}
void pkt_writeBaseMasterServerMultipartMessage(uint8_t **pkt, struct BaseMasterServerMultipartMessage in) {
	pkt_writeBaseMasterServerReliableRequest(pkt, in.base);
	pkt_writeUint32(pkt, in.multipartMessageId);
	pkt_writeVarUint32(pkt, in.offset);
	pkt_writeVarUint32(pkt, in.length);
	pkt_writeVarUint32(pkt, in.totalLength);
	pkt_writeBytes(pkt, in.data, in.length);
}
/*struct SongPackMask pkt_readSongPackMask(uint8_t **pkt) {
	struct SongPackMask out;
	out.top = pkt_readUint64(pkt);
	out.bottom = pkt_readUint64(pkt);
	return out;
}*/
/*struct BeatmapLevelSelectionMask pkt_readBeatmapLevelSelectionMask(uint8_t **pkt) {
	struct BeatmapLevelSelectionMask out;
	out.beatmapDifficultyMask = pkt_readUint8(pkt);
	out.gameplayModifiersMask = pkt_readUint32(pkt);
	out.songPackMask = pkt_readSongPackMask(pkt);
	return out;
}*/
/*struct GameplayServerConfiguration pkt_readGameplayServerConfiguration(uint8_t **pkt) {
	struct GameplayServerConfiguration out;
	out.maxPlayerCount = pkt_readVarInt32(pkt);
	out.discoveryPolicy = pkt_readVarInt32(pkt);
	out.invitePolicy = pkt_readVarInt32(pkt);
	out.gameplayServerMode = pkt_readVarInt32(pkt);
	out.songSelectionMode = pkt_readVarInt32(pkt);
	out.gameplayServerControlSettings = pkt_readVarInt32(pkt);
	return out;
}*/
/*struct BaseConnectToServerRequest pkt_readBaseConnectToServerRequest(uint8_t **pkt) {
	struct BaseConnectToServerRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(pkt);
	out.userId = pkt_readString(pkt);
	out.userName = pkt_readString(pkt);
	pkt_readBytesTo(pkt, out.random, 32);
	out.publicKey = pkt_readByteArrayNetSerializable(pkt);
	return out;
}*/
/*struct ConnectToServerRequest pkt_readConnectToServerRequest(uint8_t **pkt) {
	struct ConnectToServerRequest out;
	out.base = pkt_readBaseConnectToServerRequest(pkt);
	out.selectionMask = pkt_readBeatmapLevelSelectionMask(pkt);
	out.secret = pkt_readString(pkt);
	out.code = pkt_readString(pkt);
	out.configuration = pkt_readGameplayServerConfiguration(pkt);
	return out;
}*/
