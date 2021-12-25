/* 
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 */

#include "enum_reflection.h"
#include "packets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
uint8_t pkt_readUint8(const uint8_t **pkt) {
	uint8_t v = (*pkt)[0];
	*pkt += sizeof(v);
	return v;
}
uint16_t pkt_readUint16(const uint8_t **pkt) {
	uint16_t v = (*pkt)[0] | (*pkt)[1] << 8;
	*pkt += sizeof(v);
	return v;
}
uint32_t pkt_readUint32(const uint8_t **pkt) {
	uint32_t v = (*pkt)[0] | (*pkt)[1] << 8 | (*pkt)[2] << 16 | (*pkt)[3] << 24;
	*pkt += sizeof(v);
	return v;
}
uint64_t pkt_readUint64(const uint8_t **pkt) {
	uint64_t v = (uint64_t)(*pkt)[0] | (uint64_t)(*pkt)[1] << 8 | (uint64_t)(*pkt)[2] << 16 | (uint64_t)(*pkt)[3] << 24 | (uint64_t)(*pkt)[4] << 32 | (uint64_t)(*pkt)[5] << 40 | (uint64_t)(*pkt)[6] << 48 | (uint64_t)(*pkt)[7] << 56;
	*pkt += sizeof(v);
	return v;
}
/*uint64_t pkt_readVarUint64(const uint8_t **pkt) {
	uint64_t byte, value = 0;
	uint32_t shift = 0;
	for(; (byte = (uint64_t)pkt_readUint8(pkt)) & 128; shift += 7)
		value |= (byte & 127) << shift;
	return value | byte << shift;
}*/
uint64_t pkt_readVarUint64(const uint8_t **pkt) {
	uint64_t byte, value = 0;
	uint8_t shift = 0;
	do {
		byte = pkt_readUint8(pkt);
		value |= (byte & 127) << shift;
		shift += 7;
	} while(byte & 128);
	return value;
}
int64_t pkt_readVarInt64(const uint8_t **pkt) {
	int64_t varULong = (int64_t)pkt_readVarUint64(pkt);
	if((varULong & 1L) != 1L)
		return varULong >> 1;
	return -(varULong >> 1) + 1L;
}
uint32_t pkt_readVarUint32(const uint8_t **pkt) {
	return (uint32_t)pkt_readVarUint64(pkt);
}
int32_t pkt_readVarInt32(const uint8_t **pkt) {
	return (int32_t)pkt_readVarInt64(pkt);
}
#define pkt_readInt8Array(pkt, out, count) pkt_readUint8Array(pkt, (uint8_t*)out, count)
void pkt_readUint8Array(const uint8_t **pkt, uint8_t *out, uint32_t count) {
	memcpy(out, *pkt, count);
	*pkt += count;
}
void pkt_writeUint8(uint8_t **pkt, uint8_t v) {
	(*pkt)[0] = v;
	*pkt += sizeof(v);
}
void pkt_writeUint16(uint8_t **pkt, uint16_t v) {
	(*pkt)[0] = v & 255;
	(*pkt)[1] = v >> 8 & 255;
	*pkt += sizeof(v);
}
void pkt_writeUint32(uint8_t **pkt, uint32_t v) {
	(*pkt)[0] = v & 255;
	(*pkt)[1] = v >> 8 & 255;
	(*pkt)[2] = v >> 16 & 255;
	(*pkt)[3] = v >> 24 & 255;
	*pkt += sizeof(v);
}
void pkt_writeUint64(uint8_t **pkt, uint64_t v) {
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
void pkt_writeVarUint64(uint8_t **pkt, uint64_t v) {
	do {
		uint8_t byte = v & 127;
		v >>= 7;
		if(v)
			byte |= 128;
		pkt_writeUint8(pkt, byte);
	} while(v);
}
void pkt_writeVarInt64(uint8_t **pkt, int64_t v) {
	if(v < 0)
		return pkt_writeVarUint64(pkt, (-(v + 1L) << 1) + 1L);
	pkt_writeVarUint64(pkt, v << 1);
}
void pkt_writeVarUint32(uint8_t **pkt, uint32_t v) {
	pkt_writeVarUint64(pkt, v);
}
void pkt_writeVarInt32(uint8_t **pkt, int32_t v) {
	pkt_writeVarInt64(pkt, v);
}
void pkt_writeUint8Array(uint8_t **pkt, uint8_t *in, uint32_t count) {
	memcpy(*pkt, in, count);
	*pkt += count;
}
#define pkt_logUint8(name, buf, it, v) pkt_logUint64(name, buf, it, (uint64_t)v)
#define pkt_logUint16(name, buf, it, v) pkt_logUint64(name, buf, it, (uint64_t)v)
#define pkt_logUint32(name, buf, it, v) pkt_logUint64(name, buf, it, (uint64_t)v)
#define pkt_logInt32(name, buf, it, v) pkt_logInt64(name, buf, it, (int64_t)v)
#define pkt_logVarUint64 pkt_logUint64
#define pkt_logVarInt64 pkt_logInt64
#define pkt_logVarUint32 pkt_logUint32
#define pkt_logVarInt32 pkt_logInt32
static void pkt_logUint64(const char *name, char *buf, char *it, uint64_t v) {
	fprintf(stderr, "%.*s%s=%" PRIu64 "\n", (uint32_t)(it - buf), buf, name, v);
}
static void pkt_logInt64(const char *name, char *buf, char *it, int64_t v) {
	fprintf(stderr, "%.*s%s=%" PRId64 "\n", (uint32_t)(it - buf), buf, name, v);
}
#define pkt_logInt8Array(name, buf, it, in, count) pkt_logUint8Array(name, buf, it, (uint8_t*)in, count)
static void pkt_logUint8Array(const char *name, char *buf, char *it, uint8_t *in, uint32_t count) {
	fprintf(stderr, "%.*s%s=", (uint32_t)(it - buf), buf, name);
	for(uint32_t i = 0; i < count; ++i)
		fprintf(stderr, "%02hhx", in[i]);
	fprintf(stderr, "\n");
}
static const uint8_t NetPacketHeaderSize[32] = {
	1, 4, 4, 3, 11, 14, 11, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};
struct NetPacketHeader pkt_readNetPacketHeader(const uint8_t **pkt) {
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
static struct ServerCode pkt_readServerCode(const uint8_t **pkt) {
	static const uint8_t table[128] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24,25,26,27,28,0,29,30,31,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,0,8,9,10,11,12,0,13,14,15,16,17,18,19,20,21,22,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	struct ServerCode out = {0};
	struct String scode = pkt_readString(pkt);
	for(uint32_t i = 0; i < 5; ++i, out.value <<= 5)
		out.value |= table[scode.data[i] & 127];
	if(scode.length != 5)
		out.value = 0xffffffff;
	return out;
}
static void pkt_writeServerCode(uint8_t **pkt, struct ServerCode in) {
	struct String scode = {.length = 5};
	for(uint32_t i = 0, code = in.value; i < 5; ++i, code >>= 5)
		scode.data[i] = "ABCDEFGHJKLMNPQRSTUVWXYZ12345789"[code & 31];
	pkt_writeString(pkt, scode);
}
static void pkt_logServerCode(const char *name, char *buf, char *it, struct ServerCode in) {
	fprintf(stderr, "%.*s%s=\"", (uint32_t)(it - buf), buf, name);
	for(uint32_t i = 0, code = in.value; i < 5; ++i, code >>= 5)
		fprintf(stderr, "%c", "ABCDEFGHJKLMNPQRSTUVWXYZ12345789"[code & 31]);
	fprintf(stderr, "\"\n");
}
static void pkt_logString(const char *name, char *buf, char *it, struct String in) {
	fprintf(stderr, "%.*s%s=\"%.*s\"\n", (uint32_t)(it - buf), buf, name, in.length, in.data);
}
struct PacketEncryptionLayer pkt_readPacketEncryptionLayer(const uint8_t **pkt) {
	struct PacketEncryptionLayer out;
	out.encrypted = pkt_readUint8(pkt);
	if(out.encrypted == 1) {
		out.sequenceId = pkt_readUint32(pkt);
		pkt_readUint8Array(pkt, out.iv, 16);
	}
	return out;
}
struct MessageHeader pkt_readMessageHeader(const uint8_t **pkt) {
	struct MessageHeader out;
	out.type = pkt_readUint32(pkt);
	out.protocolVersion = pkt_readVarUint32(pkt);
	return out;
}
struct SerializeHeader pkt_readSerializeHeader(const uint8_t **pkt) {
	struct SerializeHeader out;
	out.length = pkt_readVarUint32(pkt);
	out.type = pkt_readUint8(pkt);
	return out;
}
struct BaseMasterServerReliableRequest pkt_readBaseMasterServerReliableRequest(const uint8_t **pkt) {
	struct BaseMasterServerReliableRequest out;
	out.requestId = pkt_readUint32(pkt);
	return out;
}
struct BaseMasterServerResponse pkt_readBaseMasterServerResponse(const uint8_t **pkt) {
	struct BaseMasterServerResponse out;
	out.responseId = pkt_readUint32(pkt);
	return out;
}
struct BaseMasterServerReliableResponse pkt_readBaseMasterServerReliableResponse(const uint8_t **pkt) {
	struct BaseMasterServerReliableResponse out;
	out.requestId = pkt_readUint32(pkt);
	out.responseId = pkt_readUint32(pkt);
	return out;
}
struct BaseMasterServerAcknowledgeMessage pkt_readBaseMasterServerAcknowledgeMessage(const uint8_t **pkt) {
	struct BaseMasterServerAcknowledgeMessage out;
	out.base = pkt_readBaseMasterServerResponse(pkt);
	out.messageHandled = pkt_readUint8(pkt);
	return out;
}
struct ByteArrayNetSerializable pkt_readByteArrayNetSerializable(const uint8_t **pkt) {
	struct ByteArrayNetSerializable out;
	out.length = pkt_readVarUint32(pkt);
	if(out.length > 4096) {
		fprintf(stderr, "Buffer overflow in read of ByteArrayNetSerializable.data: %u > 4096\n", (uint32_t)out.length);
		out.length = 0;
	}
	pkt_readUint8Array(pkt, out.data, out.length);
	return out;
}
struct String pkt_readString(const uint8_t **pkt) {
	struct String out;
	out.length = pkt_readUint32(pkt);
	if(out.length > 4096) {
		fprintf(stderr, "Buffer overflow in read of String.data: %u > 4096\n", (uint32_t)out.length);
		out.length = 0;
	}
	pkt_readInt8Array(pkt, out.data, out.length);
	return out;
}
struct AuthenticationToken pkt_readAuthenticationToken(const uint8_t **pkt) {
	struct AuthenticationToken out;
	out.platform = pkt_readUint8(pkt);
	out.userId = pkt_readString(pkt);
	out.userName = pkt_readString(pkt);
	out.sessionToken = pkt_readByteArrayNetSerializable(pkt);
	return out;
}
struct BaseMasterServerMultipartMessage pkt_readBaseMasterServerMultipartMessage(const uint8_t **pkt) {
	struct BaseMasterServerMultipartMessage out;
	out.base = pkt_readBaseMasterServerReliableRequest(pkt);
	out.multipartMessageId = pkt_readUint32(pkt);
	out.offset = pkt_readVarUint32(pkt);
	out.length = pkt_readVarUint32(pkt);
	out.totalLength = pkt_readVarUint32(pkt);
	if(out.length > 384) {
		fprintf(stderr, "Buffer overflow in read of BaseMasterServerMultipartMessage.data: %u > 384\n", (uint32_t)out.length);
		out.length = 0;
	}
	pkt_readUint8Array(pkt, out.data, out.length);
	return out;
}
struct BitMask128 pkt_readBitMask128(const uint8_t **pkt) {
	struct BitMask128 out;
	out._d0 = pkt_readUint64(pkt);
	out._d1 = pkt_readUint64(pkt);
	return out;
}
struct SongPackMask pkt_readSongPackMask(const uint8_t **pkt) {
	struct SongPackMask out;
	out._bloomFilter = pkt_readBitMask128(pkt);
	return out;
}
struct BeatmapLevelSelectionMask pkt_readBeatmapLevelSelectionMask(const uint8_t **pkt) {
	struct BeatmapLevelSelectionMask out;
	out.difficulties = pkt_readUint8(pkt);
	out.modifiers = pkt_readUint32(pkt);
	out.songPacks = pkt_readSongPackMask(pkt);
	return out;
}
struct GameplayServerConfiguration pkt_readGameplayServerConfiguration(const uint8_t **pkt) {
	struct GameplayServerConfiguration out;
	out.maxPlayerCount = pkt_readVarInt32(pkt);
	out.discoveryPolicy = pkt_readVarInt32(pkt);
	out.invitePolicy = pkt_readVarInt32(pkt);
	out.gameplayServerMode = pkt_readVarInt32(pkt);
	out.songSelectionMode = pkt_readVarInt32(pkt);
	out.gameplayServerControlSettings = pkt_readVarInt32(pkt);
	return out;
}
struct PublicServerInfo pkt_readPublicServerInfo(const uint8_t **pkt) {
	struct PublicServerInfo out;
	out.code = pkt_readServerCode(pkt);
	out.currentPlayerCount = pkt_readVarInt32(pkt);
	return out;
}
struct IPEndPoint pkt_readIPEndPoint(const uint8_t **pkt) {
	struct IPEndPoint out;
	out.address = pkt_readString(pkt);
	out.port = pkt_readUint32(pkt);
	return out;
}
struct AuthenticateUserRequest pkt_readAuthenticateUserRequest(const uint8_t **pkt) {
	struct AuthenticateUserRequest out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	out.authenticationToken = pkt_readAuthenticationToken(pkt);
	return out;
}
struct AuthenticateUserResponse pkt_readAuthenticateUserResponse(const uint8_t **pkt) {
	struct AuthenticateUserResponse out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	out.result = pkt_readUint8(pkt);
	return out;
}
struct ConnectToServerResponse pkt_readConnectToServerResponse(const uint8_t **pkt) {
	struct ConnectToServerResponse out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	out.result = pkt_readUint8(pkt);
	if(out.result == GetPublicServersResponse_Result_Success) {
		out.userId = pkt_readString(pkt);
		out.userName = pkt_readString(pkt);
		out.secret = pkt_readString(pkt);
		out.selectionMask = pkt_readBeatmapLevelSelectionMask(pkt);
		out.flags = pkt_readUint8(pkt);
		out.remoteEndPoint = pkt_readIPEndPoint(pkt);
		pkt_readUint8Array(pkt, out.random, 32);
		out.publicKey = pkt_readByteArrayNetSerializable(pkt);
		out.code = pkt_readString(pkt);
		out.configuration = pkt_readGameplayServerConfiguration(pkt);
		out.managerId = pkt_readString(pkt);
	}
	return out;
}
struct ConnectToServerRequest pkt_readConnectToServerRequest(const uint8_t **pkt) {
	struct ConnectToServerRequest out;
	out = (struct ConnectToServerRequest){};
	return out;
}
struct UserMessageReceivedAcknowledge pkt_readUserMessageReceivedAcknowledge(const uint8_t **pkt) {
	struct UserMessageReceivedAcknowledge out;
	out.base = pkt_readBaseMasterServerAcknowledgeMessage(pkt);
	return out;
}
struct UserMultipartMessage pkt_readUserMultipartMessage(const uint8_t **pkt) {
	struct UserMultipartMessage out;
	out.base = pkt_readBaseMasterServerMultipartMessage(pkt);
	return out;
}
struct SessionKeepaliveMessage pkt_readSessionKeepaliveMessage(const uint8_t **pkt) {
	struct SessionKeepaliveMessage out;
	out = (struct SessionKeepaliveMessage){};
	return out;
}
struct GetPublicServersRequest pkt_readGetPublicServersRequest(const uint8_t **pkt) {
	struct GetPublicServersRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(pkt);
	out.userId = pkt_readString(pkt);
	out.userName = pkt_readString(pkt);
	out.offset = pkt_readVarInt32(pkt);
	out.count = pkt_readVarInt32(pkt);
	out.selectionMask = pkt_readBeatmapLevelSelectionMask(pkt);
	out.configuration = pkt_readGameplayServerConfiguration(pkt);
	return out;
}
struct GetPublicServersResponse pkt_readGetPublicServersResponse(const uint8_t **pkt) {
	struct GetPublicServersResponse out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	out.result = pkt_readUint8(pkt);
	if(out.result == GetPublicServersResponse_Result_Success) {
		out.publicServerCount = pkt_readVarUint32(pkt);
		if(out.publicServerCount > 16384) {
			fprintf(stderr, "Buffer overflow in read of GetPublicServersResponse.publicServers: %u > 16384\n", (uint32_t)out.publicServerCount);
			out.publicServerCount = 0;
		}
		for(uint32_t i = 0; i < out.publicServerCount; ++i)
			out.publicServers[i] = pkt_readPublicServerInfo(pkt);
	}
	return out;
}
struct AuthenticateDedicatedServerRequest pkt_readAuthenticateDedicatedServerRequest(const uint8_t **pkt) {
	struct AuthenticateDedicatedServerRequest out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	out.dedicatedServerId = pkt_readString(pkt);
	pkt_readUint8Array(pkt, out.nonce, 16);
	pkt_readUint8Array(pkt, out.hash, 32);
	out.timestamp = pkt_readUint64(pkt);
	return out;
}
struct AuthenticateDedicatedServerResponse pkt_readAuthenticateDedicatedServerResponse(const uint8_t **pkt) {
	struct AuthenticateDedicatedServerResponse out;
	out = (struct AuthenticateDedicatedServerResponse){};
	return out;
}
struct CreateDedicatedServerInstanceRequest pkt_readCreateDedicatedServerInstanceRequest(const uint8_t **pkt) {
	struct CreateDedicatedServerInstanceRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(pkt);
	out.secret = pkt_readString(pkt);
	out.selectionMask = pkt_readBeatmapLevelSelectionMask(pkt);
	out.userId = pkt_readString(pkt);
	out.userName = pkt_readString(pkt);
	out.userEndPoint = pkt_readIPEndPoint(pkt);
	pkt_readUint8Array(pkt, out.userRandom, 32);
	out.userPublicKey = pkt_readByteArrayNetSerializable(pkt);
	out.configuration = pkt_readGameplayServerConfiguration(pkt);
	return out;
}
struct CreateDedicatedServerInstanceResponse pkt_readCreateDedicatedServerInstanceResponse(const uint8_t **pkt) {
	struct CreateDedicatedServerInstanceResponse out;
	out = (struct CreateDedicatedServerInstanceResponse){};
	return out;
}
struct DedicatedServerInstanceNoLongerAvailableRequest pkt_readDedicatedServerInstanceNoLongerAvailableRequest(const uint8_t **pkt) {
	struct DedicatedServerInstanceNoLongerAvailableRequest out;
	out = (struct DedicatedServerInstanceNoLongerAvailableRequest){};
	return out;
}
struct DedicatedServerHeartbeatRequest pkt_readDedicatedServerHeartbeatRequest(const uint8_t **pkt) {
	struct DedicatedServerHeartbeatRequest out;
	out = (struct DedicatedServerHeartbeatRequest){};
	return out;
}
struct DedicatedServerHeartbeatResponse pkt_readDedicatedServerHeartbeatResponse(const uint8_t **pkt) {
	struct DedicatedServerHeartbeatResponse out;
	out = (struct DedicatedServerHeartbeatResponse){};
	return out;
}
struct DedicatedServerInstanceStatusUpdateRequest pkt_readDedicatedServerInstanceStatusUpdateRequest(const uint8_t **pkt) {
	struct DedicatedServerInstanceStatusUpdateRequest out;
	out = (struct DedicatedServerInstanceStatusUpdateRequest){};
	return out;
}
struct DedicatedServerShutDownRequest pkt_readDedicatedServerShutDownRequest(const uint8_t **pkt) {
	struct DedicatedServerShutDownRequest out;
	out = (struct DedicatedServerShutDownRequest){};
	return out;
}
struct DedicatedServerPrepareForConnectionRequest pkt_readDedicatedServerPrepareForConnectionRequest(const uint8_t **pkt) {
	struct DedicatedServerPrepareForConnectionRequest out;
	out = (struct DedicatedServerPrepareForConnectionRequest){};
	return out;
}
struct DedicatedServerMessageReceivedAcknowledge pkt_readDedicatedServerMessageReceivedAcknowledge(const uint8_t **pkt) {
	struct DedicatedServerMessageReceivedAcknowledge out;
	out.base = pkt_readBaseMasterServerAcknowledgeMessage(pkt);
	return out;
}
struct DedicatedServerMultipartMessage pkt_readDedicatedServerMultipartMessage(const uint8_t **pkt) {
	struct DedicatedServerMultipartMessage out;
	out.base = pkt_readBaseMasterServerMultipartMessage(pkt);
	return out;
}
struct DedicatedServerPrepareForConnectionResponse pkt_readDedicatedServerPrepareForConnectionResponse(const uint8_t **pkt) {
	struct DedicatedServerPrepareForConnectionResponse out;
	out = (struct DedicatedServerPrepareForConnectionResponse){};
	return out;
}
struct ClientHelloRequest pkt_readClientHelloRequest(const uint8_t **pkt) {
	struct ClientHelloRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(pkt);
	pkt_readUint8Array(pkt, out.random, 32);
	return out;
}
struct HelloVerifyRequest pkt_readHelloVerifyRequest(const uint8_t **pkt) {
	struct HelloVerifyRequest out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	pkt_readUint8Array(pkt, out.cookie, 32);
	return out;
}
struct ClientHelloWithCookieRequest pkt_readClientHelloWithCookieRequest(const uint8_t **pkt) {
	struct ClientHelloWithCookieRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(pkt);
	out.certificateResponseId = pkt_readUint32(pkt);
	pkt_readUint8Array(pkt, out.random, 32);
	pkt_readUint8Array(pkt, out.cookie, 32);
	return out;
}
struct ServerHelloRequest pkt_readServerHelloRequest(const uint8_t **pkt) {
	struct ServerHelloRequest out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	pkt_readUint8Array(pkt, out.random, 32);
	out.publicKey = pkt_readByteArrayNetSerializable(pkt);
	out.signature = pkt_readByteArrayNetSerializable(pkt);
	return out;
}
struct ServerCertificateRequest pkt_readServerCertificateRequest(const uint8_t **pkt) {
	struct ServerCertificateRequest out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	out.certificateCount = pkt_readVarUint32(pkt);
	if(out.certificateCount > 10) {
		fprintf(stderr, "Buffer overflow in read of ServerCertificateRequest.certificateList: %u > 10\n", (uint32_t)out.certificateCount);
		out.certificateCount = 0;
	}
	for(uint32_t i = 0; i < out.certificateCount; ++i)
		out.certificateList[i] = pkt_readByteArrayNetSerializable(pkt);
	return out;
}
struct ServerCertificateResponse pkt_readServerCertificateResponse(const uint8_t **pkt) {
	struct ServerCertificateResponse out;
	out = (struct ServerCertificateResponse){};
	return out;
}
struct ClientKeyExchangeRequest pkt_readClientKeyExchangeRequest(const uint8_t **pkt) {
	struct ClientKeyExchangeRequest out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	out.clientPublicKey = pkt_readByteArrayNetSerializable(pkt);
	return out;
}
struct ChangeCipherSpecRequest pkt_readChangeCipherSpecRequest(const uint8_t **pkt) {
	struct ChangeCipherSpecRequest out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	return out;
}
struct HandshakeMessageReceivedAcknowledge pkt_readHandshakeMessageReceivedAcknowledge(const uint8_t **pkt) {
	struct HandshakeMessageReceivedAcknowledge out;
	out.base = pkt_readBaseMasterServerAcknowledgeMessage(pkt);
	return out;
}
struct HandshakeMultipartMessage pkt_readHandshakeMultipartMessage(const uint8_t **pkt) {
	struct HandshakeMultipartMessage out;
	out.base = pkt_readBaseMasterServerMultipartMessage(pkt);
	return out;
}
void pkt_writePacketEncryptionLayer(uint8_t **pkt, struct PacketEncryptionLayer in) {
	pkt_writeUint8(pkt, in.encrypted);
	if(in.encrypted == 1) {
		pkt_writeUint32(pkt, in.sequenceId);
		pkt_writeUint8Array(pkt, in.iv, 16);
	}
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
	pkt_writeUint8Array(pkt, in.data, in.length);
}
void pkt_writeString(uint8_t **pkt, struct String in) {
	pkt_writeUint32(pkt, in.length);
	pkt_writeInt8Array(pkt, in.data, in.length);
}
void pkt_writeBaseMasterServerMultipartMessage(uint8_t **pkt, struct BaseMasterServerMultipartMessage in) {
	pkt_writeBaseMasterServerReliableRequest(pkt, in.base);
	pkt_writeUint32(pkt, in.multipartMessageId);
	pkt_writeVarUint32(pkt, in.offset);
	pkt_writeVarUint32(pkt, in.length);
	pkt_writeVarUint32(pkt, in.totalLength);
	pkt_writeUint8Array(pkt, in.data, in.length);
}
void pkt_writeBitMask128(uint8_t **pkt, struct BitMask128 in) {
	pkt_writeUint64(pkt, in._d0);
	pkt_writeUint64(pkt, in._d1);
}
void pkt_writeSongPackMask(uint8_t **pkt, struct SongPackMask in) {
	pkt_writeBitMask128(pkt, in._bloomFilter);
}
void pkt_writeBeatmapLevelSelectionMask(uint8_t **pkt, struct BeatmapLevelSelectionMask in) {
	pkt_writeUint8(pkt, in.difficulties);
	pkt_writeUint32(pkt, in.modifiers);
	pkt_writeSongPackMask(pkt, in.songPacks);
}
void pkt_writeGameplayServerConfiguration(uint8_t **pkt, struct GameplayServerConfiguration in) {
	pkt_writeVarInt32(pkt, in.maxPlayerCount);
	pkt_writeVarInt32(pkt, in.discoveryPolicy);
	pkt_writeVarInt32(pkt, in.invitePolicy);
	pkt_writeVarInt32(pkt, in.gameplayServerMode);
	pkt_writeVarInt32(pkt, in.songSelectionMode);
	pkt_writeVarInt32(pkt, in.gameplayServerControlSettings);
}
void pkt_writePublicServerInfo(uint8_t **pkt, struct PublicServerInfo in) {
	pkt_writeServerCode(pkt, in.code);
	pkt_writeVarInt32(pkt, in.currentPlayerCount);
}
void pkt_writeIPEndPoint(uint8_t **pkt, struct IPEndPoint in) {
	pkt_writeString(pkt, in.address);
	pkt_writeUint32(pkt, in.port);
}
void pkt_writeAuthenticateUserResponse(uint8_t **pkt, struct AuthenticateUserResponse in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeUint8(pkt, in.result);
}
void pkt_writeConnectToServerResponse(uint8_t **pkt, struct ConnectToServerResponse in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeUint8(pkt, in.result);
	if(in.result == GetPublicServersResponse_Result_Success) {
		pkt_writeString(pkt, in.userId);
		pkt_writeString(pkt, in.userName);
		pkt_writeString(pkt, in.secret);
		pkt_writeBeatmapLevelSelectionMask(pkt, in.selectionMask);
		pkt_writeUint8(pkt, in.flags);
		pkt_writeIPEndPoint(pkt, in.remoteEndPoint);
		pkt_writeUint8Array(pkt, in.random, 32);
		pkt_writeByteArrayNetSerializable(pkt, in.publicKey);
		pkt_writeString(pkt, in.code);
		pkt_writeGameplayServerConfiguration(pkt, in.configuration);
		pkt_writeString(pkt, in.managerId);
	}
}
void pkt_writeConnectToServerRequest(uint8_t **pkt, struct ConnectToServerRequest in) {
}
void pkt_writeUserMessageReceivedAcknowledge(uint8_t **pkt, struct UserMessageReceivedAcknowledge in) {
	pkt_writeBaseMasterServerAcknowledgeMessage(pkt, in.base);
}
void pkt_writeUserMultipartMessage(uint8_t **pkt, struct UserMultipartMessage in) {
	pkt_writeBaseMasterServerMultipartMessage(pkt, in.base);
}
void pkt_writeSessionKeepaliveMessage(uint8_t **pkt, struct SessionKeepaliveMessage in) {
}
void pkt_writeGetPublicServersResponse(uint8_t **pkt, struct GetPublicServersResponse in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeUint8(pkt, in.result);
	if(in.result == GetPublicServersResponse_Result_Success) {
		pkt_writeVarUint32(pkt, in.publicServerCount);
		for(uint32_t i = 0; i < in.publicServerCount; ++i)
			pkt_writePublicServerInfo(pkt, in.publicServers[i]);
	}
}
void pkt_writeAuthenticateDedicatedServerResponse(uint8_t **pkt, struct AuthenticateDedicatedServerResponse in) {
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
	pkt_writeBaseMasterServerAcknowledgeMessage(pkt, in.base);
}
void pkt_writeDedicatedServerMultipartMessage(uint8_t **pkt, struct DedicatedServerMultipartMessage in) {
	pkt_writeBaseMasterServerMultipartMessage(pkt, in.base);
}
void pkt_writeDedicatedServerPrepareForConnectionResponse(uint8_t **pkt, struct DedicatedServerPrepareForConnectionResponse in) {
}
void pkt_writeHelloVerifyRequest(uint8_t **pkt, struct HelloVerifyRequest in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeUint8Array(pkt, in.cookie, 32);
}
void pkt_writeServerHelloRequest(uint8_t **pkt, struct ServerHelloRequest in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeUint8Array(pkt, in.random, 32);
	pkt_writeByteArrayNetSerializable(pkt, in.publicKey);
	pkt_writeByteArrayNetSerializable(pkt, in.signature);
}
void pkt_writeServerCertificateRequest(uint8_t **pkt, struct ServerCertificateRequest in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeVarUint32(pkt, in.certificateCount);
	for(uint32_t i = 0; i < in.certificateCount; ++i)
		pkt_writeByteArrayNetSerializable(pkt, in.certificateList[i]);
}
void pkt_writeChangeCipherSpecRequest(uint8_t **pkt, struct ChangeCipherSpecRequest in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
}
void pkt_writeHandshakeMessageReceivedAcknowledge(uint8_t **pkt, struct HandshakeMessageReceivedAcknowledge in) {
	pkt_writeBaseMasterServerAcknowledgeMessage(pkt, in.base);
}
void pkt_writeHandshakeMultipartMessage(uint8_t **pkt, struct HandshakeMultipartMessage in) {
	pkt_writeBaseMasterServerMultipartMessage(pkt, in.base);
}
void pkt_logPacketEncryptionLayer(const char *name, char *buf, char *it, struct PacketEncryptionLayer in) {
	it += sprintf(it, "%s.", name);
	pkt_logUint8("encrypted", buf, it, in.encrypted);
	if(in.encrypted == 1) {
		pkt_logUint32("sequenceId", buf, it, in.sequenceId);
		pkt_logUint8Array("iv", buf, it, in.iv, 16);
	}
}
void pkt_logMessageHeader(const char *name, char *buf, char *it, struct MessageHeader in) {
	it += sprintf(it, "%s.", name);
	fprintf(stderr, "%stype=%s\n", buf, reflect(MessageType, in.type));
	pkt_logVarUint32("protocolVersion", buf, it, in.protocolVersion);
}
void pkt_logSerializeHeader(const char *name, char *buf, char *it, struct SerializeHeader in) {
	it += sprintf(it, "%s.", name);
	pkt_logVarUint32("length", buf, it, in.length);
	pkt_logUint8("type", buf, it, in.type);
}
void pkt_logBaseMasterServerReliableRequest(const char *name, char *buf, char *it, struct BaseMasterServerReliableRequest in) {
	it += sprintf(it, "%s.", name);
	pkt_logUint32("requestId", buf, it, in.requestId);
}
void pkt_logBaseMasterServerResponse(const char *name, char *buf, char *it, struct BaseMasterServerResponse in) {
	it += sprintf(it, "%s.", name);
	pkt_logUint32("responseId", buf, it, in.responseId);
}
void pkt_logBaseMasterServerReliableResponse(const char *name, char *buf, char *it, struct BaseMasterServerReliableResponse in) {
	it += sprintf(it, "%s.", name);
	pkt_logUint32("requestId", buf, it, in.requestId);
	pkt_logUint32("responseId", buf, it, in.responseId);
}
void pkt_logBaseMasterServerAcknowledgeMessage(const char *name, char *buf, char *it, struct BaseMasterServerAcknowledgeMessage in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerResponse("base", buf, it, in.base);
	pkt_logUint8("messageHandled", buf, it, in.messageHandled);
}
void pkt_logByteArrayNetSerializable(const char *name, char *buf, char *it, struct ByteArrayNetSerializable in) {
	it += sprintf(it, "%s.", name);
	pkt_logVarUint32("length", buf, it, in.length);
	pkt_logUint8Array("data", buf, it, in.data, in.length);
}
void pkt_logAuthenticationToken(const char *name, char *buf, char *it, struct AuthenticationToken in) {
	it += sprintf(it, "%s.", name);
	fprintf(stderr, "%splatform=%s\n", buf, reflect(Platform, in.platform));
	pkt_logString("userId", buf, it, in.userId);
	pkt_logString("userName", buf, it, in.userName);
	pkt_logByteArrayNetSerializable("sessionToken", buf, it, in.sessionToken);
}
void pkt_logBaseMasterServerMultipartMessage(const char *name, char *buf, char *it, struct BaseMasterServerMultipartMessage in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableRequest("base", buf, it, in.base);
	pkt_logUint32("multipartMessageId", buf, it, in.multipartMessageId);
	pkt_logVarUint32("offset", buf, it, in.offset);
	pkt_logVarUint32("length", buf, it, in.length);
	pkt_logVarUint32("totalLength", buf, it, in.totalLength);
	pkt_logUint8Array("data", buf, it, in.data, in.length);
}
void pkt_logBitMask128(const char *name, char *buf, char *it, struct BitMask128 in) {
	it += sprintf(it, "%s.", name);
	pkt_logUint64("_d0", buf, it, in._d0);
	pkt_logUint64("_d1", buf, it, in._d1);
}
void pkt_logSongPackMask(const char *name, char *buf, char *it, struct SongPackMask in) {
	it += sprintf(it, "%s.", name);
	pkt_logBitMask128("_bloomFilter", buf, it, in._bloomFilter);
}
void pkt_logBeatmapLevelSelectionMask(const char *name, char *buf, char *it, struct BeatmapLevelSelectionMask in) {
	it += sprintf(it, "%s.", name);
	fprintf(stderr, "%sdifficulties=%s\n", buf, reflect(BeatmapDifficultyMask, in.difficulties));
	fprintf(stderr, "%smodifiers=%s\n", buf, reflect(GameplayModifierMask, in.modifiers));
	pkt_logSongPackMask("songPacks", buf, it, in.songPacks);
}
void pkt_logGameplayServerConfiguration(const char *name, char *buf, char *it, struct GameplayServerConfiguration in) {
	it += sprintf(it, "%s.", name);
	pkt_logVarInt32("maxPlayerCount", buf, it, in.maxPlayerCount);
	fprintf(stderr, "%sdiscoveryPolicy=%s\n", buf, reflect(DiscoveryPolicy, in.discoveryPolicy));
	fprintf(stderr, "%sinvitePolicy=%s\n", buf, reflect(InvitePolicy, in.invitePolicy));
	fprintf(stderr, "%sgameplayServerMode=%s\n", buf, reflect(GameplayServerMode, in.gameplayServerMode));
	fprintf(stderr, "%ssongSelectionMode=%s\n", buf, reflect(SongSelectionMode, in.songSelectionMode));
	fprintf(stderr, "%sgameplayServerControlSettings=%s\n", buf, reflect(GameplayServerControlSettings, in.gameplayServerControlSettings));
}
void pkt_logPublicServerInfo(const char *name, char *buf, char *it, struct PublicServerInfo in) {
	it += sprintf(it, "%s.", name);
	pkt_logServerCode("code", buf, it, in.code);
	pkt_logVarInt32("currentPlayerCount", buf, it, in.currentPlayerCount);
}
void pkt_logIPEndPoint(const char *name, char *buf, char *it, struct IPEndPoint in) {
	it += sprintf(it, "%s.", name);
	pkt_logString("address", buf, it, in.address);
	pkt_logUint32("port", buf, it, in.port);
}
void pkt_logAuthenticateUserRequest(const char *name, char *buf, char *it, struct AuthenticateUserRequest in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableResponse("base", buf, it, in.base);
	pkt_logAuthenticationToken("authenticationToken", buf, it, in.authenticationToken);
}
void pkt_logAuthenticateUserResponse(const char *name, char *buf, char *it, struct AuthenticateUserResponse in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableResponse("base", buf, it, in.base);
	fprintf(stderr, "%sresult=%s\n", buf, reflect(AuthenticateUserResponse_Result, in.result));
}
void pkt_logConnectToServerResponse(const char *name, char *buf, char *it, struct ConnectToServerResponse in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableResponse("base", buf, it, in.base);
	fprintf(stderr, "%sresult=%s\n", buf, reflect(ConnectToServerResponse_Result, in.result));
	if(in.result == GetPublicServersResponse_Result_Success) {
		pkt_logString("userId", buf, it, in.userId);
		pkt_logString("userName", buf, it, in.userName);
		pkt_logString("secret", buf, it, in.secret);
		pkt_logBeatmapLevelSelectionMask("selectionMask", buf, it, in.selectionMask);
		pkt_logUint8("flags", buf, it, in.flags);
		pkt_logIPEndPoint("remoteEndPoint", buf, it, in.remoteEndPoint);
		pkt_logUint8Array("random", buf, it, in.random, 32);
		pkt_logByteArrayNetSerializable("publicKey", buf, it, in.publicKey);
		pkt_logString("code", buf, it, in.code);
		pkt_logGameplayServerConfiguration("configuration", buf, it, in.configuration);
		pkt_logString("managerId", buf, it, in.managerId);
	}
}
void pkt_logConnectToServerRequest(const char *name, char *buf, char *it, struct ConnectToServerRequest in) {
	it += sprintf(it, "%s.", name);
}
void pkt_logUserMessageReceivedAcknowledge(const char *name, char *buf, char *it, struct UserMessageReceivedAcknowledge in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerAcknowledgeMessage("base", buf, it, in.base);
}
void pkt_logUserMultipartMessage(const char *name, char *buf, char *it, struct UserMultipartMessage in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerMultipartMessage("base", buf, it, in.base);
}
void pkt_logSessionKeepaliveMessage(const char *name, char *buf, char *it, struct SessionKeepaliveMessage in) {
	it += sprintf(it, "%s.", name);
}
void pkt_logGetPublicServersRequest(const char *name, char *buf, char *it, struct GetPublicServersRequest in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableRequest("base", buf, it, in.base);
	pkt_logString("userId", buf, it, in.userId);
	pkt_logString("userName", buf, it, in.userName);
	pkt_logVarInt32("offset", buf, it, in.offset);
	pkt_logVarInt32("count", buf, it, in.count);
	pkt_logBeatmapLevelSelectionMask("selectionMask", buf, it, in.selectionMask);
	pkt_logGameplayServerConfiguration("configuration", buf, it, in.configuration);
}
void pkt_logGetPublicServersResponse(const char *name, char *buf, char *it, struct GetPublicServersResponse in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableResponse("base", buf, it, in.base);
	fprintf(stderr, "%sresult=%s\n", buf, reflect(GetPublicServersResponse_Result, in.result));
	if(in.result == GetPublicServersResponse_Result_Success) {
		pkt_logVarUint32("publicServerCount", buf, it, in.publicServerCount);
		for(uint32_t i = 0; i < in.publicServerCount; ++i)
			pkt_logPublicServerInfo("publicServers", buf, it, in.publicServers[i]);
	}
}
void pkt_logAuthenticateDedicatedServerRequest(const char *name, char *buf, char *it, struct AuthenticateDedicatedServerRequest in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableResponse("base", buf, it, in.base);
	pkt_logString("dedicatedServerId", buf, it, in.dedicatedServerId);
	pkt_logUint8Array("nonce", buf, it, in.nonce, 16);
	pkt_logUint8Array("hash", buf, it, in.hash, 32);
	pkt_logUint64("timestamp", buf, it, in.timestamp);
}
void pkt_logAuthenticateDedicatedServerResponse(const char *name, char *buf, char *it, struct AuthenticateDedicatedServerResponse in) {
	it += sprintf(it, "%s.", name);
}
void pkt_logCreateDedicatedServerInstanceRequest(const char *name, char *buf, char *it, struct CreateDedicatedServerInstanceRequest in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableRequest("base", buf, it, in.base);
	pkt_logString("secret", buf, it, in.secret);
	pkt_logBeatmapLevelSelectionMask("selectionMask", buf, it, in.selectionMask);
	pkt_logString("userId", buf, it, in.userId);
	pkt_logString("userName", buf, it, in.userName);
	pkt_logIPEndPoint("userEndPoint", buf, it, in.userEndPoint);
	pkt_logUint8Array("userRandom", buf, it, in.userRandom, 32);
	pkt_logByteArrayNetSerializable("userPublicKey", buf, it, in.userPublicKey);
	pkt_logGameplayServerConfiguration("configuration", buf, it, in.configuration);
}
void pkt_logCreateDedicatedServerInstanceResponse(const char *name, char *buf, char *it, struct CreateDedicatedServerInstanceResponse in) {
	it += sprintf(it, "%s.", name);
}
void pkt_logDedicatedServerInstanceNoLongerAvailableRequest(const char *name, char *buf, char *it, struct DedicatedServerInstanceNoLongerAvailableRequest in) {
	it += sprintf(it, "%s.", name);
}
void pkt_logDedicatedServerHeartbeatRequest(const char *name, char *buf, char *it, struct DedicatedServerHeartbeatRequest in) {
	it += sprintf(it, "%s.", name);
}
void pkt_logDedicatedServerHeartbeatResponse(const char *name, char *buf, char *it, struct DedicatedServerHeartbeatResponse in) {
	it += sprintf(it, "%s.", name);
}
void pkt_logDedicatedServerInstanceStatusUpdateRequest(const char *name, char *buf, char *it, struct DedicatedServerInstanceStatusUpdateRequest in) {
	it += sprintf(it, "%s.", name);
}
void pkt_logDedicatedServerShutDownRequest(const char *name, char *buf, char *it, struct DedicatedServerShutDownRequest in) {
	it += sprintf(it, "%s.", name);
}
void pkt_logDedicatedServerPrepareForConnectionRequest(const char *name, char *buf, char *it, struct DedicatedServerPrepareForConnectionRequest in) {
	it += sprintf(it, "%s.", name);
}
void pkt_logDedicatedServerMessageReceivedAcknowledge(const char *name, char *buf, char *it, struct DedicatedServerMessageReceivedAcknowledge in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerAcknowledgeMessage("base", buf, it, in.base);
}
void pkt_logDedicatedServerMultipartMessage(const char *name, char *buf, char *it, struct DedicatedServerMultipartMessage in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerMultipartMessage("base", buf, it, in.base);
}
void pkt_logDedicatedServerPrepareForConnectionResponse(const char *name, char *buf, char *it, struct DedicatedServerPrepareForConnectionResponse in) {
	it += sprintf(it, "%s.", name);
}
void pkt_logClientHelloRequest(const char *name, char *buf, char *it, struct ClientHelloRequest in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableRequest("base", buf, it, in.base);
	pkt_logUint8Array("random", buf, it, in.random, 32);
}
void pkt_logHelloVerifyRequest(const char *name, char *buf, char *it, struct HelloVerifyRequest in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableResponse("base", buf, it, in.base);
	pkt_logUint8Array("cookie", buf, it, in.cookie, 32);
}
void pkt_logClientHelloWithCookieRequest(const char *name, char *buf, char *it, struct ClientHelloWithCookieRequest in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableRequest("base", buf, it, in.base);
	pkt_logUint32("certificateResponseId", buf, it, in.certificateResponseId);
	pkt_logUint8Array("random", buf, it, in.random, 32);
	pkt_logUint8Array("cookie", buf, it, in.cookie, 32);
}
void pkt_logServerHelloRequest(const char *name, char *buf, char *it, struct ServerHelloRequest in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableResponse("base", buf, it, in.base);
	pkt_logUint8Array("random", buf, it, in.random, 32);
	pkt_logByteArrayNetSerializable("publicKey", buf, it, in.publicKey);
	pkt_logByteArrayNetSerializable("signature", buf, it, in.signature);
}
void pkt_logServerCertificateRequest(const char *name, char *buf, char *it, struct ServerCertificateRequest in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableResponse("base", buf, it, in.base);
	pkt_logVarUint32("certificateCount", buf, it, in.certificateCount);
	for(uint32_t i = 0; i < in.certificateCount; ++i)
		pkt_logByteArrayNetSerializable("certificateList", buf, it, in.certificateList[i]);
}
void pkt_logServerCertificateResponse(const char *name, char *buf, char *it, struct ServerCertificateResponse in) {
	it += sprintf(it, "%s.", name);
}
void pkt_logClientKeyExchangeRequest(const char *name, char *buf, char *it, struct ClientKeyExchangeRequest in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableResponse("base", buf, it, in.base);
	pkt_logByteArrayNetSerializable("clientPublicKey", buf, it, in.clientPublicKey);
}
void pkt_logChangeCipherSpecRequest(const char *name, char *buf, char *it, struct ChangeCipherSpecRequest in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerReliableResponse("base", buf, it, in.base);
}
void pkt_logHandshakeMessageReceivedAcknowledge(const char *name, char *buf, char *it, struct HandshakeMessageReceivedAcknowledge in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerAcknowledgeMessage("base", buf, it, in.base);
}
void pkt_logHandshakeMultipartMessage(const char *name, char *buf, char *it, struct HandshakeMultipartMessage in) {
	it += sprintf(it, "%s.", name);
	pkt_logBaseMasterServerMultipartMessage("base", buf, it, in.base);
}
