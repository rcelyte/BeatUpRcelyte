/* 
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 */

#include "enum_reflection.h"
#include "packets.h"
static const uint8_t _trap[128] = {~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,};
#include "scramble.h"
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
		if(shift >= 64) {
			fprintf(stderr, "Buffer overflow in read of VarUint\n");
			*pkt = _trap;
			return 0;
		}
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
float pkt_readFloat32(const uint8_t **pkt) {
	float v = *(const float*)*pkt;
	*pkt += 4;
	return v;
}
double pkt_readFloat64(const uint8_t **pkt) {
	double v = *(const double*)*pkt;
	*pkt += 8;
	return v;
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
		pkt_writeVarUint64(pkt, (-(v + 1L) << 1) + 1L);
	else
		pkt_writeVarUint64(pkt, v << 1);
}
void pkt_writeVarUint32(uint8_t **pkt, uint32_t v) {
	pkt_writeVarUint64(pkt, v);
}
void pkt_writeVarInt32(uint8_t **pkt, int32_t v) {
	pkt_writeVarInt64(pkt, v);
}
void pkt_writeUint8Array(uint8_t **pkt, const uint8_t *in, uint32_t count) {
	memcpy(*pkt, in, count);
	*pkt += count;
}
void pkt_writeFloat32(uint8_t **pkt, float v) {
	*(float*)*pkt = v;
	*pkt += 4;
}
void pkt_writeFloat64(uint8_t **pkt, double v) {
	*(double*)*pkt = v;
	*pkt += 8;
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
#define pkt_logInt8Array(name, buf, it, in, count) pkt_logUint8Array(name, buf, it, (const uint8_t*)in, count)
static void pkt_logUint8Array(const char *name, char *buf, char *it, const uint8_t *in, uint32_t count) {
	fprintf(stderr, "%.*s%s=", (uint32_t)(it - buf), buf, name);
	for(uint32_t i = 0; i < count; ++i)
		fprintf(stderr, "%02hhx", in[i]);
	fprintf(stderr, "\n");
}
#define pkt_logFloat32 pkt_logFloat64
static void pkt_logFloat64(const char *name, char *buf, char *it, double v) {
	fprintf(stderr, "%.*s%s=%f\n", (uint32_t)(it - buf), buf, name, v);
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
void pkt_writePacketEncryptionLayer(uint8_t **pkt, struct PacketEncryptionLayer in) {
	pkt_writeUint8(pkt, in.encrypted);
	if(in.encrypted == 1) {
		pkt_writeUint32(pkt, in.sequenceId);
		pkt_writeUint8Array(pkt, in.iv, 16);
	}
}
struct BaseMasterServerReliableRequest pkt_readBaseMasterServerReliableRequest(const uint8_t **pkt) {
	struct BaseMasterServerReliableRequest out;
	out.requestId = pkt_readUint32(pkt);
	return out;
}
void pkt_writeBaseMasterServerReliableRequest(uint8_t **pkt, struct BaseMasterServerReliableRequest in) {
	pkt_writeUint32(pkt, in.requestId);
}
struct BaseMasterServerResponse pkt_readBaseMasterServerResponse(const uint8_t **pkt) {
	struct BaseMasterServerResponse out;
	out.responseId = pkt_readUint32(pkt);
	return out;
}
void pkt_writeBaseMasterServerResponse(uint8_t **pkt, struct BaseMasterServerResponse in) {
	pkt_writeUint32(pkt, in.responseId);
}
struct BaseMasterServerReliableResponse pkt_readBaseMasterServerReliableResponse(const uint8_t **pkt) {
	struct BaseMasterServerReliableResponse out;
	out.requestId = pkt_readUint32(pkt);
	out.responseId = pkt_readUint32(pkt);
	return out;
}
void pkt_writeBaseMasterServerReliableResponse(uint8_t **pkt, struct BaseMasterServerReliableResponse in) {
	pkt_writeUint32(pkt, in.requestId);
	pkt_writeUint32(pkt, in.responseId);
}
struct BaseMasterServerAcknowledgeMessage pkt_readBaseMasterServerAcknowledgeMessage(const uint8_t **pkt) {
	struct BaseMasterServerAcknowledgeMessage out;
	out.base = pkt_readBaseMasterServerResponse(pkt);
	out.messageHandled = pkt_readUint8(pkt);
	return out;
}
void pkt_writeBaseMasterServerAcknowledgeMessage(uint8_t **pkt, struct BaseMasterServerAcknowledgeMessage in) {
	pkt_writeBaseMasterServerResponse(pkt, in.base);
	pkt_writeUint8(pkt, in.messageHandled);
}
struct ByteArrayNetSerializable pkt_readByteArrayNetSerializable(const uint8_t **pkt) {
	struct ByteArrayNetSerializable out;
	out.length = pkt_readVarUint32(pkt);
	if(out.length > 4096) {
		fprintf(stderr, "Buffer overflow in read of ByteArrayNetSerializable.data: %u > 4096\n", (uint32_t)out.length), out.length = 0, *pkt = _trap;
	} else {
		pkt_readUint8Array(pkt, out.data, out.length);
	}
	return out;
}
void pkt_writeByteArrayNetSerializable(uint8_t **pkt, struct ByteArrayNetSerializable in) {
	pkt_writeVarUint32(pkt, in.length);
	pkt_writeUint8Array(pkt, in.data, in.length);
}
#ifdef PACKET_LOGGING_FUNCS
static void pkt_logString(const char *name, char *buf, char *it, struct String in) {
	fprintf(stderr, "%.*s%s=\"%.*s\"\n", (uint32_t)(it - buf), buf, name, in.length, in.data);
}
static void pkt_logLongString(const char *name, char *buf, char *it, struct LongString in) {
	fprintf(stderr, "%.*s%s=\"%.*s\"\n", (uint32_t)(it - buf), buf, name, in.length, in.data);
}
#endif
struct String pkt_readString(const uint8_t **pkt) {
	struct String out;
	out.length = pkt_readUint32(pkt);
	if(out.length > 60) {
		fprintf(stderr, "Buffer overflow in read of String.data: %u > 60\n", (uint32_t)out.length), out.length = 0, *pkt = _trap;
	} else {
		pkt_readInt8Array(pkt, out.data, out.length);
	}
	return out;
}
void pkt_writeString(uint8_t **pkt, struct String in) {
	pkt_writeUint32(pkt, in.length);
	pkt_writeInt8Array(pkt, in.data, in.length);
}
struct LongString pkt_readLongString(const uint8_t **pkt) {
	struct LongString out;
	out.length = pkt_readUint32(pkt);
	if(out.length > 4096) {
		fprintf(stderr, "Buffer overflow in read of LongString.data: %u > 4096\n", (uint32_t)out.length), out.length = 0, *pkt = _trap;
	} else {
		pkt_readInt8Array(pkt, out.data, out.length);
	}
	return out;
}
void pkt_writeLongString(uint8_t **pkt, struct LongString in) {
	pkt_writeUint32(pkt, in.length);
	pkt_writeInt8Array(pkt, in.data, in.length);
}
struct AuthenticationToken pkt_readAuthenticationToken(const uint8_t **pkt) {
	struct AuthenticationToken out;
	out.platform = pkt_readUint8(pkt);
	out.userId = pkt_readString(pkt);
	out.userName = pkt_readString(pkt);
	out.sessionToken = pkt_readByteArrayNetSerializable(pkt);
	return out;
}
void pkt_writeBaseMasterServerMultipartMessage(uint8_t **pkt, struct BaseMasterServerMultipartMessage in) {
	pkt_writeBaseMasterServerReliableRequest(pkt, in.base);
	pkt_writeUint32(pkt, in.multipartMessageId);
	pkt_writeVarUint32(pkt, in.offset);
	pkt_writeVarUint32(pkt, in.length);
	pkt_writeVarUint32(pkt, in.totalLength);
	pkt_writeUint8Array(pkt, in.data, in.length);
}
struct BitMask128 pkt_readBitMask128(const uint8_t **pkt) {
	struct BitMask128 out;
	out.d0 = pkt_readUint64(pkt);
	out.d1 = pkt_readUint64(pkt);
	return out;
}
void pkt_writeBitMask128(uint8_t **pkt, struct BitMask128 in) {
	pkt_writeUint64(pkt, in.d0);
	pkt_writeUint64(pkt, in.d1);
}
struct SongPackMask pkt_readSongPackMask(const uint8_t **pkt) {
	struct SongPackMask out;
	out.bloomFilter = pkt_readBitMask128(pkt);
	return out;
}
void pkt_writeSongPackMask(uint8_t **pkt, struct SongPackMask in) {
	pkt_writeBitMask128(pkt, in.bloomFilter);
}
struct BeatmapLevelSelectionMask pkt_readBeatmapLevelSelectionMask(const uint8_t **pkt) {
	struct BeatmapLevelSelectionMask out;
	out.difficulties = pkt_readUint8(pkt);
	out.modifiers = pkt_readUint32(pkt);
	out.songPacks = pkt_readSongPackMask(pkt);
	return out;
}
void pkt_writeBeatmapLevelSelectionMask(uint8_t **pkt, struct BeatmapLevelSelectionMask in) {
	pkt_writeUint8(pkt, in.difficulties);
	pkt_writeUint32(pkt, in.modifiers);
	pkt_writeSongPackMask(pkt, in.songPacks);
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
void pkt_writeGameplayServerConfiguration(uint8_t **pkt, struct GameplayServerConfiguration in) {
	pkt_writeVarInt32(pkt, in.maxPlayerCount);
	pkt_writeVarInt32(pkt, in.discoveryPolicy);
	pkt_writeVarInt32(pkt, in.invitePolicy);
	pkt_writeVarInt32(pkt, in.gameplayServerMode);
	pkt_writeVarInt32(pkt, in.songSelectionMode);
	pkt_writeVarInt32(pkt, in.gameplayServerControlSettings);
}
ServerCode StringToServerCode(const char *in, uint32_t len) {
	static const uint8_t readTable[128] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,27,28,29,30,31,32,33,34,35,36,1,1,1,1,1,1,1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	ServerCode out = 0;
	if(len <= 5)
		for(uint32_t i = 0, fac = 1; i < len; ++i, fac *= 36)
			out += readTable[in[i] & 127] * fac;
	return scramble_decode(out);
}
char *ServerCodeToString(char *out, ServerCode in) {
	char *s = out;
	for(in = scramble_encode(in); in; in /= 36)
		*s++ = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[--in % 36];
	*s = 0;
	return out;
}
static ServerCode pkt_readServerCode(const uint8_t **pkt) {
	struct String str = pkt_readString(pkt);
	return StringToServerCode(str.data, str.length);
}
static void pkt_writeServerCode(uint8_t **pkt, ServerCode in) {
	struct String str = {.length = 0};
	for(in = scramble_encode(in); in; in /= 36)
		str.data[str.length++] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[--in % 36];
	pkt_writeString(pkt, str);
}
#ifdef PACKET_LOGGING_FUNCS
static void pkt_logServerCode(const char *name, char *buf, char *it, ServerCode in) {
	fprintf(stderr, "%.*s%s=%u (\"", (uint32_t)(it - buf), buf, name, in);
	for(; in; in /= 36)
		fprintf(stderr, "%c", "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[--in % 36]);
	fprintf(stderr, "\")\n");
}
#endif
void pkt_writeIPEndPoint(uint8_t **pkt, struct IPEndPoint in) {
	pkt_writeString(pkt, in.address);
	pkt_writeUint32(pkt, in.port);
}
struct BaseConnectToServerRequest pkt_readBaseConnectToServerRequest(const uint8_t **pkt) {
	struct BaseConnectToServerRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(pkt);
	out.userId = pkt_readString(pkt);
	out.userName = pkt_readString(pkt);
	pkt_readUint8Array(pkt, out.random, 32);
	out.publicKey = pkt_readByteArrayNetSerializable(pkt);
	return out;
}
struct Channeled pkt_readChanneled(const uint8_t **pkt) {
	struct Channeled out;
	out.sequence = pkt_readUint16(pkt);
	out.channelId = pkt_readUint8(pkt);
	return out;
}
void pkt_writeChanneled(uint8_t **pkt, struct Channeled in) {
	pkt_writeUint16(pkt, in.sequence);
	pkt_writeUint8(pkt, in.channelId);
}
struct Ack pkt_readAck(const uint8_t **pkt) {
	struct Ack out;
	out.sequence = pkt_readUint16(pkt);
	out.channelId = pkt_readUint8(pkt);
	if(out.channelId % 2 == 0) {
		pkt_readUint8Array(pkt, out.data, 9);
	}
	return out;
}
void pkt_writeAck(uint8_t **pkt, struct Ack in) {
	pkt_writeUint16(pkt, in.sequence);
	pkt_writeUint8(pkt, in.channelId);
	if(in.channelId % 2 == 0) {
		pkt_writeUint8Array(pkt, in.data, 9);
	}
}
struct Ping pkt_readPing(const uint8_t **pkt) {
	struct Ping out;
	out.sequence = pkt_readUint16(pkt);
	return out;
}
void pkt_writePong(uint8_t **pkt, struct Pong in) {
	pkt_writeUint16(pkt, in.sequence);
	pkt_writeUint64(pkt, in.time);
}
struct ConnectRequest pkt_readConnectRequest(const uint8_t **pkt) {
	struct ConnectRequest out;
	out.protocolId = pkt_readUint32(pkt);
	out.connectId = pkt_readUint64(pkt);
	out.addrlen = pkt_readUint8(pkt);
	if(out.addrlen > 38) {
		fprintf(stderr, "Buffer overflow in read of ConnectRequest.address: %u > 38\n", (uint32_t)out.addrlen), out.addrlen = 0, *pkt = _trap;
	} else {
		pkt_readUint8Array(pkt, out.address, out.addrlen);
	}
	out.secret = pkt_readString(pkt);
	out.userId = pkt_readString(pkt);
	out.userName = pkt_readString(pkt);
	out.isConnectionOwner = pkt_readUint8(pkt);
	return out;
}
void pkt_writeConnectAccept(uint8_t **pkt, struct ConnectAccept in) {
	pkt_writeUint64(pkt, in.connectId);
	pkt_writeUint8(pkt, in.connectNum);
	pkt_writeUint8(pkt, in.reusedPeer);
}
struct Disconnect pkt_readDisconnect(const uint8_t **pkt) {
	struct Disconnect out;
	pkt_readUint8Array(pkt, out._pad0, 8);
	return out;
}
struct MtuCheck pkt_readMtuCheck(const uint8_t **pkt) {
	struct MtuCheck out;
	out.newMtu0 = pkt_readUint32(pkt);
	if(out.newMtu0-9 > 1423) {
		fprintf(stderr, "Buffer overflow in read of MtuCheck.pad: %u > 1423\n", (uint32_t)out.newMtu0-9), out.newMtu0 = 0, *pkt = _trap;
	} else {
		pkt_readUint8Array(pkt, out.pad, out.newMtu0-9);
	}
	out.newMtu1 = pkt_readUint32(pkt);
	return out;
}
void pkt_writeMtuOk(uint8_t **pkt, struct MtuOk in) {
	pkt_writeUint32(pkt, in.newMtu0);
	pkt_writeUint8Array(pkt, in.pad, in.newMtu0-9);
	pkt_writeUint32(pkt, in.newMtu1);
}
struct NetPacketHeader pkt_readNetPacketHeader(const uint8_t **pkt) {
	struct NetPacketHeader out;
	uint8_t bits = pkt_readUint8(pkt);
	out.property = (bits >> 0) & 31;
	out.connectionNumber = (bits >> 5) & 3;
	out.isFragmented = (bits >> 7) & 1;
	return out;
}
void pkt_writeNetPacketHeader(uint8_t **pkt, struct NetPacketHeader in) {
	uint8_t bits = 0;
	bits |= (in.property << 0);
	bits |= (in.connectionNumber << 5);
	bits |= (in.isFragmented << 7);
	pkt_writeUint8(pkt, bits);
}
struct FragmentedHeader pkt_readFragmentedHeader(const uint8_t **pkt) {
	struct FragmentedHeader out;
	out.fragmentId = pkt_readUint16(pkt);
	out.fragmentPart = pkt_readUint16(pkt);
	out.fragmentsTotal = pkt_readUint16(pkt);
	return out;
}
struct PlayerStateHash pkt_readPlayerStateHash(const uint8_t **pkt) {
	struct PlayerStateHash out;
	out.bloomFilter = pkt_readBitMask128(pkt);
	return out;
}
void pkt_writePlayerStateHash(uint8_t **pkt, struct PlayerStateHash in) {
	pkt_writeBitMask128(pkt, in.bloomFilter);
}
struct Color32 pkt_readColor32(const uint8_t **pkt) {
	struct Color32 out;
	out.r = pkt_readUint8(pkt);
	out.g = pkt_readUint8(pkt);
	out.b = pkt_readUint8(pkt);
	out.a = pkt_readUint8(pkt);
	return out;
}
void pkt_writeColor32(uint8_t **pkt, struct Color32 in) {
	pkt_writeUint8(pkt, in.r);
	pkt_writeUint8(pkt, in.g);
	pkt_writeUint8(pkt, in.b);
	pkt_writeUint8(pkt, in.a);
}
struct MultiplayerAvatarData pkt_readMultiplayerAvatarData(const uint8_t **pkt) {
	struct MultiplayerAvatarData out;
	out.headTopId = pkt_readString(pkt);
	out.headTopPrimaryColor = pkt_readColor32(pkt);
	out.handsColor = pkt_readColor32(pkt);
	out.clothesId = pkt_readString(pkt);
	out.clothesPrimaryColor = pkt_readColor32(pkt);
	out.clothesSecondaryColor = pkt_readColor32(pkt);
	out.clothesDetailColor = pkt_readColor32(pkt);
	for(uint32_t i = 0; i < 2; ++i)
		out._unused[i] = pkt_readColor32(pkt);
	out.eyesId = pkt_readString(pkt);
	out.mouthId = pkt_readString(pkt);
	out.glassesColor = pkt_readColor32(pkt);
	out.facialHairColor = pkt_readColor32(pkt);
	out.headTopSecondaryColor = pkt_readColor32(pkt);
	out.glassesId = pkt_readString(pkt);
	out.facialHairId = pkt_readString(pkt);
	out.handsId = pkt_readString(pkt);
	return out;
}
void pkt_writeMultiplayerAvatarData(uint8_t **pkt, struct MultiplayerAvatarData in) {
	pkt_writeString(pkt, in.headTopId);
	pkt_writeColor32(pkt, in.headTopPrimaryColor);
	pkt_writeColor32(pkt, in.handsColor);
	pkt_writeString(pkt, in.clothesId);
	pkt_writeColor32(pkt, in.clothesPrimaryColor);
	pkt_writeColor32(pkt, in.clothesSecondaryColor);
	pkt_writeColor32(pkt, in.clothesDetailColor);
	for(uint32_t i = 0; i < 2; ++i)
		pkt_writeColor32(pkt, in._unused[i]);
	pkt_writeString(pkt, in.eyesId);
	pkt_writeString(pkt, in.mouthId);
	pkt_writeColor32(pkt, in.glassesColor);
	pkt_writeColor32(pkt, in.facialHairColor);
	pkt_writeColor32(pkt, in.headTopSecondaryColor);
	pkt_writeString(pkt, in.glassesId);
	pkt_writeString(pkt, in.facialHairId);
	pkt_writeString(pkt, in.handsId);
}
struct RoutingHeader pkt_readRoutingHeader(const uint8_t **pkt) {
	struct RoutingHeader out;
	out.remoteConnectionId = pkt_readUint8(pkt);
	uint8_t bits = pkt_readUint8(pkt);
	out.connectionId = (bits >> 0) & 127;
	out.encrypted = (bits >> 7) & 1;
	return out;
}
void pkt_writeRoutingHeader(uint8_t **pkt, struct RoutingHeader in) {
	pkt_writeUint8(pkt, in.remoteConnectionId);
	uint8_t bits = 0;
	bits |= (in.connectionId << 0);
	bits |= (in.encrypted << 7);
	pkt_writeUint8(pkt, bits);
}
void pkt_writeSyncTime(uint8_t **pkt, struct SyncTime in) {
	pkt_writeFloat32(pkt, in.syncTime);
}
void pkt_writePlayerConnected(uint8_t **pkt, struct PlayerConnected in) {
	pkt_writeUint8(pkt, in.remoteConnectionId);
	pkt_writeString(pkt, in.userId);
	pkt_writeString(pkt, in.userName);
	pkt_writeUint8(pkt, in.isConnectionOwner);
}
struct PlayerIdentity pkt_readPlayerIdentity(const uint8_t **pkt) {
	struct PlayerIdentity out;
	out.playerState = pkt_readPlayerStateHash(pkt);
	out.playerAvatar = pkt_readMultiplayerAvatarData(pkt);
	out.random = pkt_readByteArrayNetSerializable(pkt);
	out.publicEncryptionKey = pkt_readByteArrayNetSerializable(pkt);
	return out;
}
void pkt_writePlayerIdentity(uint8_t **pkt, struct PlayerIdentity in) {
	pkt_writePlayerStateHash(pkt, in.playerState);
	pkt_writeMultiplayerAvatarData(pkt, in.playerAvatar);
	pkt_writeByteArrayNetSerializable(pkt, in.random);
	pkt_writeByteArrayNetSerializable(pkt, in.publicEncryptionKey);
}
void pkt_writePlayerDisconnected(uint8_t **pkt, struct PlayerDisconnected in) {
	pkt_writeVarInt32(pkt, in.disconnectedReason);
}
void pkt_writePlayerSortOrderUpdate(uint8_t **pkt, struct PlayerSortOrderUpdate in) {
	pkt_writeString(pkt, in.userId);
	pkt_writeVarInt32(pkt, in.sortIndex);
}
struct PlayerStateUpdate pkt_readPlayerStateUpdate(const uint8_t **pkt) {
	struct PlayerStateUpdate out;
	out.playerState = pkt_readPlayerStateHash(pkt);
	return out;
}
struct PingMessage pkt_readPingMessage(const uint8_t **pkt) {
	struct PingMessage out;
	out.pingTime = pkt_readFloat32(pkt);
	return out;
}
struct PongMessage pkt_readPongMessage(const uint8_t **pkt) {
	struct PongMessage out;
	out.pingTime = pkt_readFloat32(pkt);
	return out;
}
struct RemoteProcedureCall pkt_readRemoteProcedureCall(const uint8_t **pkt) {
	struct RemoteProcedureCall out;
	out.syncTime = pkt_readFloat32(pkt);
	return out;
}
void pkt_writeRemoteProcedureCall(uint8_t **pkt, struct RemoteProcedureCall in) {
	pkt_writeFloat32(pkt, in.syncTime);
}
struct RemoteProcedureCallFlags pkt_readRemoteProcedureCallFlags(const uint8_t **pkt, uint32_t protocolVersion) {
	struct RemoteProcedureCallFlags out;
	uint8_t bits = 255;
	if(protocolVersion > 6)
		bits = pkt_readUint8(pkt);
	out.hasValue0 = (bits >> 0) & 1;
	out.hasValue1 = (bits >> 1) & 1;
	out.hasValue2 = (bits >> 2) & 1;
	out.hasValue3 = (bits >> 3) & 1;
	return out;
}
void pkt_writeRemoteProcedureCallFlags(uint8_t **pkt, struct RemoteProcedureCallFlags in, uint32_t protocolVersion) {
	if(protocolVersion > 6) {
		uint8_t bits = 0;
		bits |= (in.hasValue0 << 0);
		bits |= (in.hasValue1 << 1);
		bits |= (in.hasValue2 << 2);
		bits |= (in.hasValue3 << 3);
		pkt_writeUint8(pkt, bits);
	}
}
void pkt_writePlayersMissingEntitlementsNetSerializable(uint8_t **pkt, struct PlayersMissingEntitlementsNetSerializable in) {
	pkt_writeInt32(pkt, in.count);
	for(uint32_t i = 0; i < in.count; ++i)
		pkt_writeString(pkt, in.playersWithoutEntitlements[i]);
}
struct BeatmapIdentifierNetSerializable pkt_readBeatmapIdentifierNetSerializable(const uint8_t **pkt) {
	struct BeatmapIdentifierNetSerializable out;
	out.levelID = pkt_readLongString(pkt);
	out.beatmapCharacteristicSerializedName = pkt_readString(pkt);
	out.difficulty = pkt_readVarUint32(pkt);
	return out;
}
void pkt_writeBeatmapIdentifierNetSerializable(uint8_t **pkt, struct BeatmapIdentifierNetSerializable in) {
	pkt_writeLongString(pkt, in.levelID);
	pkt_writeString(pkt, in.beatmapCharacteristicSerializedName);
	pkt_writeVarUint32(pkt, in.difficulty);
}
struct GameplayModifiers pkt_readGameplayModifiers(const uint8_t **pkt) {
	struct GameplayModifiers out;
	int32_t bits = pkt_readInt32(pkt);
	out.energyType = (bits >> 0) & 15;
	out.demoNoFail = (bits >> 5) & 1;
	out.instaFail = (bits >> 6) & 1;
	out.failOnSaberClash = (bits >> 7) & 1;
	out.enabledObstacleType = (bits >> 8) & 15;
	out.demoNoObstacles = (bits >> 12) & 1;
	out.noBombs = (bits >> 13) & 1;
	out.fastNotes = (bits >> 14) & 1;
	out.strictAngles = (bits >> 15) & 1;
	out.disappearingArrows = (bits >> 16) & 1;
	out.ghostNotes = (bits >> 17) & 1;
	out.songSpeed = (bits >> 18) & 15;
	out.noArrows = (bits >> 22) & 1;
	out.noFailOn0Energy = (bits >> 23) & 1;
	out.proMode = (bits >> 24) & 1;
	out.zenMode = (bits >> 25) & 1;
	out.smallCubes = (bits >> 26) & 1;
	return out;
}
void pkt_writeGameplayModifiers(uint8_t **pkt, struct GameplayModifiers in) {
	int32_t bits = 0;
	bits |= (in.energyType << 0);
	bits |= (in.demoNoFail << 5);
	bits |= (in.instaFail << 6);
	bits |= (in.failOnSaberClash << 7);
	bits |= (in.enabledObstacleType << 8);
	bits |= (in.demoNoObstacles << 12);
	bits |= (in.noBombs << 13);
	bits |= (in.fastNotes << 14);
	bits |= (in.strictAngles << 15);
	bits |= (in.disappearingArrows << 16);
	bits |= (in.ghostNotes << 17);
	bits |= (in.songSpeed << 18);
	bits |= (in.noArrows << 22);
	bits |= (in.noFailOn0Energy << 23);
	bits |= (in.proMode << 24);
	bits |= (in.zenMode << 25);
	bits |= (in.smallCubes << 26);
	pkt_writeInt32(pkt, bits);
}
void pkt_writePlayerLobbyPermissionConfigurationNetSerializable(uint8_t **pkt, struct PlayerLobbyPermissionConfigurationNetSerializable in) {
	pkt_writeString(pkt, in.userId);
	uint8_t bits = 0;
	bits |= (in.isServerOwner << 0);
	bits |= (in.hasRecommendBeatmapsPermission << 1);
	bits |= (in.hasRecommendGameplayModifiersPermission << 2);
	bits |= (in.hasKickVotePermission << 3);
	bits |= (in.hasInvitePermission << 4);
	pkt_writeUint8(pkt, bits);
}
void pkt_writePlayersLobbyPermissionConfigurationNetSerializable(uint8_t **pkt, struct PlayersLobbyPermissionConfigurationNetSerializable in) {
	pkt_writeInt32(pkt, in.count);
	for(uint32_t i = 0; i < in.count; ++i)
		pkt_writePlayerLobbyPermissionConfigurationNetSerializable(pkt, in.playersPermission[i]);
}
struct SyncStateId pkt_readSyncStateId(const uint8_t **pkt) {
	struct SyncStateId out;
	uint8_t bits = pkt_readUint8(pkt);
	out.id = (bits >> 0) & 127;
	out.same = (bits >> 7) & 1;
	return out;
}
struct Vector3Serializable pkt_readVector3Serializable(const uint8_t **pkt) {
	struct Vector3Serializable out;
	out.x = pkt_readVarInt32(pkt);
	out.y = pkt_readVarInt32(pkt);
	out.z = pkt_readVarInt32(pkt);
	return out;
}
struct QuaternionSerializable pkt_readQuaternionSerializable(const uint8_t **pkt) {
	struct QuaternionSerializable out;
	out.a = pkt_readVarInt32(pkt);
	out.b = pkt_readVarInt32(pkt);
	out.c = pkt_readVarInt32(pkt);
	return out;
}
struct PoseSerializable pkt_readPoseSerializable(const uint8_t **pkt) {
	struct PoseSerializable out;
	out.position = pkt_readVector3Serializable(pkt);
	out.rotation = pkt_readQuaternionSerializable(pkt);
	return out;
}
struct ColorNoAlphaSerializable pkt_readColorNoAlphaSerializable(const uint8_t **pkt) {
	struct ColorNoAlphaSerializable out;
	out.r = pkt_readFloat32(pkt);
	out.g = pkt_readFloat32(pkt);
	out.b = pkt_readFloat32(pkt);
	return out;
}
void pkt_writeColorNoAlphaSerializable(uint8_t **pkt, struct ColorNoAlphaSerializable in) {
	pkt_writeFloat32(pkt, in.r);
	pkt_writeFloat32(pkt, in.g);
	pkt_writeFloat32(pkt, in.b);
}
struct ColorSchemeNetSerializable pkt_readColorSchemeNetSerializable(const uint8_t **pkt) {
	struct ColorSchemeNetSerializable out;
	out.saberAColor = pkt_readColorNoAlphaSerializable(pkt);
	out.saberBColor = pkt_readColorNoAlphaSerializable(pkt);
	out.obstaclesColor = pkt_readColorNoAlphaSerializable(pkt);
	out.environmentColor0 = pkt_readColorNoAlphaSerializable(pkt);
	out.environmentColor1 = pkt_readColorNoAlphaSerializable(pkt);
	out.environmentColor0Boost = pkt_readColorNoAlphaSerializable(pkt);
	out.environmentColor1Boost = pkt_readColorNoAlphaSerializable(pkt);
	return out;
}
void pkt_writeColorSchemeNetSerializable(uint8_t **pkt, struct ColorSchemeNetSerializable in) {
	pkt_writeColorNoAlphaSerializable(pkt, in.saberAColor);
	pkt_writeColorNoAlphaSerializable(pkt, in.saberBColor);
	pkt_writeColorNoAlphaSerializable(pkt, in.obstaclesColor);
	pkt_writeColorNoAlphaSerializable(pkt, in.environmentColor0);
	pkt_writeColorNoAlphaSerializable(pkt, in.environmentColor1);
	pkt_writeColorNoAlphaSerializable(pkt, in.environmentColor0Boost);
	pkt_writeColorNoAlphaSerializable(pkt, in.environmentColor1Boost);
}
struct PlayerSpecificSettingsNetSerializable pkt_readPlayerSpecificSettingsNetSerializable(const uint8_t **pkt) {
	struct PlayerSpecificSettingsNetSerializable out;
	out.userId = pkt_readString(pkt);
	out.userName = pkt_readString(pkt);
	out.leftHanded = pkt_readUint8(pkt);
	out.automaticPlayerHeight = pkt_readUint8(pkt);
	out.playerHeight = pkt_readFloat32(pkt);
	out.headPosToPlayerHeightOffset = pkt_readFloat32(pkt);
	out.colorScheme = pkt_readColorSchemeNetSerializable(pkt);
	return out;
}
void pkt_writePlayerSpecificSettingsNetSerializable(uint8_t **pkt, struct PlayerSpecificSettingsNetSerializable in) {
	pkt_writeString(pkt, in.userId);
	pkt_writeString(pkt, in.userName);
	pkt_writeUint8(pkt, in.leftHanded);
	pkt_writeUint8(pkt, in.automaticPlayerHeight);
	pkt_writeFloat32(pkt, in.playerHeight);
	pkt_writeFloat32(pkt, in.headPosToPlayerHeightOffset);
	pkt_writeColorSchemeNetSerializable(pkt, in.colorScheme);
}
void pkt_writePlayerSpecificSettingsAtStartNetSerializable(uint8_t **pkt, struct PlayerSpecificSettingsAtStartNetSerializable in) {
	pkt_writeInt32(pkt, in.count);
	for(uint32_t i = 0; i < in.count; ++i)
		pkt_writePlayerSpecificSettingsNetSerializable(pkt, in.activePlayerSpecificSettingsAtGameStart[i]);
}
struct NoteCutInfoNetSerializable pkt_readNoteCutInfoNetSerializable(const uint8_t **pkt) {
	struct NoteCutInfoNetSerializable out;
	uint8_t bits = pkt_readUint8(pkt);
	out.cutWasOk = (bits >> 0) & 1;
	out.saberSpeed = pkt_readFloat32(pkt);
	out.saberDir = pkt_readVector3Serializable(pkt);
	out.cutPoint = pkt_readVector3Serializable(pkt);
	out.cutNormal = pkt_readVector3Serializable(pkt);
	out.notePosition = pkt_readVector3Serializable(pkt);
	out.noteScale = pkt_readVector3Serializable(pkt);
	out.noteRotation = pkt_readQuaternionSerializable(pkt);
	out.colorType = pkt_readVarInt32(pkt);
	out.noteLineLayer = pkt_readVarInt32(pkt);
	out.noteLineIndex = pkt_readVarInt32(pkt);
	out.noteTime = pkt_readFloat32(pkt);
	out.timeToNextColorNote = pkt_readFloat32(pkt);
	out.moveVec = pkt_readVector3Serializable(pkt);
	return out;
}
struct NoteMissInfoNetSerializable pkt_readNoteMissInfoNetSerializable(const uint8_t **pkt) {
	struct NoteMissInfoNetSerializable out;
	out.colorType = pkt_readVarInt32(pkt);
	out.noteLineLayer = pkt_readVarInt32(pkt);
	out.noteLineIndex = pkt_readVarInt32(pkt);
	out.noteTime = pkt_readFloat32(pkt);
	return out;
}
struct LevelCompletionResults pkt_readLevelCompletionResults(const uint8_t **pkt) {
	struct LevelCompletionResults out;
	out.gameplayModifiers = pkt_readGameplayModifiers(pkt);
	out.modifiedScore = pkt_readVarInt32(pkt);
	out.rawScore = pkt_readVarInt32(pkt);
	out.rank = pkt_readVarInt32(pkt);
	out.fullCombo = pkt_readUint8(pkt);
	out.leftSaberMovementDistance = pkt_readFloat32(pkt);
	out.rightSaberMovementDistance = pkt_readFloat32(pkt);
	out.leftHandMovementDistance = pkt_readFloat32(pkt);
	out.rightHandMovementDistance = pkt_readFloat32(pkt);
	out.songDuration = pkt_readFloat32(pkt);
	out.levelEndStateType = pkt_readVarInt32(pkt);
	out.levelEndAction = pkt_readVarInt32(pkt);
	out.energy = pkt_readFloat32(pkt);
	out.goodCutsCount = pkt_readVarInt32(pkt);
	out.badCutsCount = pkt_readVarInt32(pkt);
	out.missedCount = pkt_readVarInt32(pkt);
	out.notGoodCount = pkt_readVarInt32(pkt);
	out.okCount = pkt_readVarInt32(pkt);
	out.averageCutScore = pkt_readVarInt32(pkt);
	out.maxCutScore = pkt_readVarInt32(pkt);
	out.averageCutDistanceRawScore = pkt_readFloat32(pkt);
	out.maxCombo = pkt_readVarInt32(pkt);
	out.minDirDeviation = pkt_readFloat32(pkt);
	out.maxDirDeviation = pkt_readFloat32(pkt);
	out.averageDirDeviation = pkt_readFloat32(pkt);
	out.minTimeDeviation = pkt_readFloat32(pkt);
	out.maxTimeDeviation = pkt_readFloat32(pkt);
	out.averageTimeDeviation = pkt_readFloat32(pkt);
	out.endSongTime = pkt_readFloat32(pkt);
	return out;
}
struct MultiplayerLevelCompletionResults pkt_readMultiplayerLevelCompletionResults(const uint8_t **pkt, uint32_t protocolVersion) {
	struct MultiplayerLevelCompletionResults out;
	if(protocolVersion <= 6) {
		out.levelEndState = pkt_readVarInt32(pkt);
	}
	if(protocolVersion > 6) {
		out.playerLevelEndState = pkt_readVarInt32(pkt);
		out.playerLevelEndReason = pkt_readVarInt32(pkt);
	}
	if((protocolVersion <= 6 && out.levelEndState < MultiplayerLevelEndState_GivenUp) || (protocolVersion > 6 && out.playerLevelEndState != MultiplayerPlayerLevelEndState_NotStarted)) {
		out.levelCompletionResults = pkt_readLevelCompletionResults(pkt);
	}
	return out;
}
struct NodePoseSyncState1 pkt_readNodePoseSyncState1(const uint8_t **pkt) {
	struct NodePoseSyncState1 out;
	out.head = pkt_readPoseSerializable(pkt);
	out.leftController = pkt_readPoseSerializable(pkt);
	out.rightController = pkt_readPoseSerializable(pkt);
	return out;
}
struct StandardScoreSyncState pkt_readStandardScoreSyncState(const uint8_t **pkt) {
	struct StandardScoreSyncState out;
	out.modifiedScore = pkt_readVarInt32(pkt);
	out.rawScore = pkt_readVarInt32(pkt);
	out.immediateMaxPossibleRawScore = pkt_readVarInt32(pkt);
	out.combo = pkt_readVarInt32(pkt);
	out.multiplier = pkt_readVarInt32(pkt);
	return out;
}
struct NoteSpawnInfoNetSerializable pkt_readNoteSpawnInfoNetSerializable(const uint8_t **pkt) {
	struct NoteSpawnInfoNetSerializable out;
	out.time = pkt_readFloat32(pkt);
	out.lineIndex = pkt_readVarInt32(pkt);
	out.noteLineLayer = pkt_readVarInt32(pkt);
	out.beforeJumpNoteLineLayer = pkt_readVarInt32(pkt);
	out.colorType = pkt_readVarInt32(pkt);
	out.cutDirection = pkt_readVarInt32(pkt);
	out.timeToNextColorNote = pkt_readFloat32(pkt);
	out.timeToPrevColorNote = pkt_readFloat32(pkt);
	out.flipLineIndex = pkt_readVarInt32(pkt);
	out.flipYSide = pkt_readVarInt32(pkt);
	out.moveStartPos = pkt_readVector3Serializable(pkt);
	out.moveEndPos = pkt_readVector3Serializable(pkt);
	out.jumpEndPos = pkt_readVector3Serializable(pkt);
	out.jumpGravity = pkt_readFloat32(pkt);
	out.moveDuration = pkt_readFloat32(pkt);
	out.jumpDuration = pkt_readFloat32(pkt);
	out.rotation = pkt_readFloat32(pkt);
	out.cutDirectionAngleOffset = pkt_readFloat32(pkt);
	return out;
}
struct ObstacleSpawnInfoNetSerializable pkt_readObstacleSpawnInfoNetSerializable(const uint8_t **pkt) {
	struct ObstacleSpawnInfoNetSerializable out;
	out.time = pkt_readFloat32(pkt);
	out.lineIndex = pkt_readVarInt32(pkt);
	out.obstacleType = pkt_readVarInt32(pkt);
	out.duration = pkt_readFloat32(pkt);
	out.width = pkt_readVarInt32(pkt);
	out.moveStartPos = pkt_readVector3Serializable(pkt);
	out.moveEndPos = pkt_readVector3Serializable(pkt);
	out.jumpEndPos = pkt_readVector3Serializable(pkt);
	out.obstacleHeight = pkt_readFloat32(pkt);
	out.moveDuration = pkt_readFloat32(pkt);
	out.jumpDuration = pkt_readFloat32(pkt);
	out.noteLinesDistance = pkt_readFloat32(pkt);
	out.rotation = pkt_readFloat32(pkt);
	return out;
}
void pkt_writeSetPlayersMissingEntitlementsToLevel(uint8_t **pkt, struct SetPlayersMissingEntitlementsToLevel in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writePlayersMissingEntitlementsNetSerializable(pkt, in.playersMissingEntitlements);
	}
}
void pkt_writeGetIsEntitledToLevel(uint8_t **pkt, struct GetIsEntitledToLevel in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writeLongString(pkt, in.levelId);
	}
}
struct SetIsEntitledToLevel pkt_readSetIsEntitledToLevel(const uint8_t **pkt, uint32_t protocolVersion) {
	struct SetIsEntitledToLevel out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(pkt, protocolVersion);
	if(out.flags.hasValue0) {
		out.levelId = pkt_readLongString(pkt);
	}
	if(out.flags.hasValue1) {
		out.entitlementStatus = pkt_readVarInt32(pkt);
	}
	return out;
}
void pkt_writeSetSelectedBeatmap(uint8_t **pkt, struct SetSelectedBeatmap in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writeBeatmapIdentifierNetSerializable(pkt, in.identifier);
	}
}
struct RecommendBeatmap pkt_readRecommendBeatmap(const uint8_t **pkt, uint32_t protocolVersion) {
	struct RecommendBeatmap out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(pkt, protocolVersion);
	if(out.flags.hasValue0) {
		out.identifier = pkt_readBeatmapIdentifierNetSerializable(pkt);
	}
	return out;
}
void pkt_writeRecommendBeatmap(uint8_t **pkt, struct RecommendBeatmap in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writeBeatmapIdentifierNetSerializable(pkt, in.identifier);
	}
}
struct ClearRecommendedBeatmap pkt_readClearRecommendedBeatmap(const uint8_t **pkt, uint32_t protocolVersion) {
	struct ClearRecommendedBeatmap out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	return out;
}
struct GetRecommendedBeatmap pkt_readGetRecommendedBeatmap(const uint8_t **pkt, uint32_t protocolVersion) {
	struct GetRecommendedBeatmap out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	return out;
}
void pkt_writeGetRecommendedBeatmap(uint8_t **pkt, struct GetRecommendedBeatmap in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
}
void pkt_writeSetSelectedGameplayModifiers(uint8_t **pkt, struct SetSelectedGameplayModifiers in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writeGameplayModifiers(pkt, in.gameplayModifiers);
	}
}
struct RecommendGameplayModifiers pkt_readRecommendGameplayModifiers(const uint8_t **pkt, uint32_t protocolVersion) {
	struct RecommendGameplayModifiers out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(pkt, protocolVersion);
	if(out.flags.hasValue0) {
		out.gameplayModifiers = pkt_readGameplayModifiers(pkt);
	}
	return out;
}
void pkt_writeRecommendGameplayModifiers(uint8_t **pkt, struct RecommendGameplayModifiers in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writeGameplayModifiers(pkt, in.gameplayModifiers);
	}
}
struct ClearRecommendedGameplayModifiers pkt_readClearRecommendedGameplayModifiers(const uint8_t **pkt, uint32_t protocolVersion) {
	struct ClearRecommendedGameplayModifiers out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	return out;
}
struct GetRecommendedGameplayModifiers pkt_readGetRecommendedGameplayModifiers(const uint8_t **pkt, uint32_t protocolVersion) {
	struct GetRecommendedGameplayModifiers out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	return out;
}
void pkt_writeGetRecommendedGameplayModifiers(uint8_t **pkt, struct GetRecommendedGameplayModifiers in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
}
void pkt_writeStartLevel(uint8_t **pkt, struct StartLevel in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writeBeatmapIdentifierNetSerializable(pkt, in.beatmapId);
	}
	if(in.flags.hasValue1) {
		pkt_writeGameplayModifiers(pkt, in.gameplayModifiers);
	}
	if(in.flags.hasValue2) {
		pkt_writeFloat32(pkt, in.startTime);
	}
}
struct GetStartedLevel pkt_readGetStartedLevel(const uint8_t **pkt, uint32_t protocolVersion) {
	struct GetStartedLevel out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	return out;
}
void pkt_writeCancelLevelStart(uint8_t **pkt, struct CancelLevelStart in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
}
struct GetMultiplayerGameState pkt_readGetMultiplayerGameState(const uint8_t **pkt, uint32_t protocolVersion) {
	struct GetMultiplayerGameState out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	return out;
}
void pkt_writeSetMultiplayerGameState(uint8_t **pkt, struct SetMultiplayerGameState in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writeVarInt32(pkt, in.lobbyState);
	}
}
struct GetIsReady pkt_readGetIsReady(const uint8_t **pkt, uint32_t protocolVersion) {
	struct GetIsReady out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	return out;
}
void pkt_writeGetIsReady(uint8_t **pkt, struct GetIsReady in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
}
struct SetIsReady pkt_readSetIsReady(const uint8_t **pkt, uint32_t protocolVersion) {
	struct SetIsReady out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(pkt, protocolVersion);
	if(out.flags.hasValue0) {
		out.isReady = pkt_readUint8(pkt);
	}
	return out;
}
void pkt_writeSetIsReady(uint8_t **pkt, struct SetIsReady in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writeUint8(pkt, in.isReady);
	}
}
struct GetIsInLobby pkt_readGetIsInLobby(const uint8_t **pkt, uint32_t protocolVersion) {
	struct GetIsInLobby out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	return out;
}
void pkt_writeGetIsInLobby(uint8_t **pkt, struct GetIsInLobby in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
}
struct SetIsInLobby pkt_readSetIsInLobby(const uint8_t **pkt, uint32_t protocolVersion) {
	struct SetIsInLobby out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(pkt, protocolVersion);
	if(out.flags.hasValue0) {
		out.isBack = pkt_readUint8(pkt);
	}
	return out;
}
void pkt_writeSetIsInLobby(uint8_t **pkt, struct SetIsInLobby in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writeUint8(pkt, in.isBack);
	}
}
struct GetCountdownEndTime pkt_readGetCountdownEndTime(const uint8_t **pkt, uint32_t protocolVersion) {
	struct GetCountdownEndTime out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	return out;
}
void pkt_writeSetCountdownEndTime(uint8_t **pkt, struct SetCountdownEndTime in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writeFloat32(pkt, in.newTime);
	}
}
void pkt_writeCancelCountdown(uint8_t **pkt, struct CancelCountdown in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
}
struct SetOwnedSongPacks pkt_readSetOwnedSongPacks(const uint8_t **pkt, uint32_t protocolVersion) {
	struct SetOwnedSongPacks out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(pkt, protocolVersion);
	if(out.flags.hasValue0) {
		out.songPackMask = pkt_readSongPackMask(pkt);
	}
	return out;
}
struct GetPermissionConfiguration pkt_readGetPermissionConfiguration(const uint8_t **pkt, uint32_t protocolVersion) {
	struct GetPermissionConfiguration out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	return out;
}
void pkt_writeSetPermissionConfiguration(uint8_t **pkt, struct SetPermissionConfiguration in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writePlayersLobbyPermissionConfigurationNetSerializable(pkt, in.playersPermissionConfiguration);
	}
}
void pkt_writeSetIsStartButtonEnabled(uint8_t **pkt, struct SetIsStartButtonEnabled in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writeVarInt32(pkt, in.reason);
	}
}
void pkt_writeSetGameplaySceneSyncFinish(uint8_t **pkt, struct SetGameplaySceneSyncFinish in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writePlayerSpecificSettingsAtStartNetSerializable(pkt, in.playersAtGameStart);
	}
	if(in.flags.hasValue1) {
		pkt_writeString(pkt, in.sessionGameId);
	}
}
struct SetGameplaySceneReady pkt_readSetGameplaySceneReady(const uint8_t **pkt, uint32_t protocolVersion) {
	struct SetGameplaySceneReady out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(pkt, protocolVersion);
	if(out.flags.hasValue0) {
		out.playerSpecificSettingsNetSerializable = pkt_readPlayerSpecificSettingsNetSerializable(pkt);
	}
	return out;
}
void pkt_writeGetGameplaySceneReady(uint8_t **pkt, struct GetGameplaySceneReady in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
}
struct SetGameplaySongReady pkt_readSetGameplaySongReady(const uint8_t **pkt, uint32_t protocolVersion) {
	struct SetGameplaySongReady out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	return out;
}
void pkt_writeGetGameplaySongReady(uint8_t **pkt, struct GetGameplaySongReady in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
}
void pkt_writeSetSongStartTime(uint8_t **pkt, struct SetSongStartTime in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(pkt, in.flags, protocolVersion);
	if(in.flags.hasValue0) {
		pkt_writeFloat32(pkt, in.startTime);
	}
}
struct NoteCut pkt_readNoteCut(const uint8_t **pkt, uint32_t protocolVersion) {
	struct NoteCut out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(pkt, protocolVersion);
	if(out.flags.hasValue0) {
		out.songTime = pkt_readFloat32(pkt);
	}
	if(out.flags.hasValue1) {
		out.noteCutInfo = pkt_readNoteCutInfoNetSerializable(pkt);
	}
	return out;
}
struct NoteMissed pkt_readNoteMissed(const uint8_t **pkt, uint32_t protocolVersion) {
	struct NoteMissed out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(pkt, protocolVersion);
	if(out.flags.hasValue0) {
		out.songTime = pkt_readFloat32(pkt);
	}
	if(out.flags.hasValue1) {
		out.noteMissInfo = pkt_readNoteMissInfoNetSerializable(pkt);
	}
	return out;
}
struct LevelFinished pkt_readLevelFinished(const uint8_t **pkt, uint32_t protocolVersion) {
	struct LevelFinished out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(pkt, protocolVersion);
	if(out.flags.hasValue0) {
		out.results = pkt_readMultiplayerLevelCompletionResults(pkt, protocolVersion);
	}
	return out;
}
void pkt_writeReturnToMenu(uint8_t **pkt, struct ReturnToMenu in, uint32_t protocolVersion) {
	pkt_writeRemoteProcedureCall(pkt, in.base);
}
struct RequestReturnToMenu pkt_readRequestReturnToMenu(const uint8_t **pkt, uint32_t protocolVersion) {
	struct RequestReturnToMenu out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	return out;
}
struct NoteSpawned pkt_readNoteSpawned(const uint8_t **pkt, uint32_t protocolVersion) {
	struct NoteSpawned out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(pkt, protocolVersion);
	if(out.flags.hasValue0) {
		out.songTime = pkt_readFloat32(pkt);
	}
	if(out.flags.hasValue1) {
		out.noteSpawnInfo = pkt_readNoteSpawnInfoNetSerializable(pkt);
	}
	return out;
}
struct ObstacleSpawned pkt_readObstacleSpawned(const uint8_t **pkt, uint32_t protocolVersion) {
	struct ObstacleSpawned out;
	out.base = pkt_readRemoteProcedureCall(pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(pkt, protocolVersion);
	if(out.flags.hasValue0) {
		out.songTime = pkt_readFloat32(pkt);
	}
	if(out.flags.hasValue1) {
		out.obstacleSpawnInfo = pkt_readObstacleSpawnInfoNetSerializable(pkt);
	}
	return out;
}
struct NodePoseSyncState pkt_readNodePoseSyncState(const uint8_t **pkt) {
	struct NodePoseSyncState out;
	out.id = pkt_readSyncStateId(pkt);
	out.time = pkt_readFloat32(pkt);
	out.state = pkt_readNodePoseSyncState1(pkt);
	return out;
}
struct ScoreSyncState pkt_readScoreSyncState(const uint8_t **pkt) {
	struct ScoreSyncState out;
	out.id = pkt_readSyncStateId(pkt);
	out.time = pkt_readFloat32(pkt);
	out.state = pkt_readStandardScoreSyncState(pkt);
	return out;
}
struct NodePoseSyncStateDelta pkt_readNodePoseSyncStateDelta(const uint8_t **pkt) {
	struct NodePoseSyncStateDelta out;
	out.baseId = pkt_readSyncStateId(pkt);
	out.timeOffsetMs = pkt_readVarInt32(pkt);
	if(out.baseId.same == 0) {
		out.delta = pkt_readNodePoseSyncState1(pkt);
	}
	return out;
}
struct ScoreSyncStateDelta pkt_readScoreSyncStateDelta(const uint8_t **pkt) {
	struct ScoreSyncStateDelta out;
	out.baseId = pkt_readSyncStateId(pkt);
	out.timeOffsetMs = pkt_readVarInt32(pkt);
	if(out.baseId.same == 0) {
		out.delta = pkt_readStandardScoreSyncState(pkt);
	}
	return out;
}
struct MpCore pkt_readMpCore(const uint8_t **pkt) {
	struct MpCore out;
	out.packetType = pkt_readString(pkt);
	return out;
}
struct MultiplayerSessionMessageHeader pkt_readMultiplayerSessionMessageHeader(const uint8_t **pkt) {
	struct MultiplayerSessionMessageHeader out;
	out.type = pkt_readUint8(pkt);
	return out;
}
void pkt_writeMultiplayerSessionMessageHeader(uint8_t **pkt, struct MultiplayerSessionMessageHeader in) {
	pkt_writeUint8(pkt, in.type);
}
struct MenuRpcHeader pkt_readMenuRpcHeader(const uint8_t **pkt) {
	struct MenuRpcHeader out;
	out.type = pkt_readUint8(pkt);
	return out;
}
void pkt_writeMenuRpcHeader(uint8_t **pkt, struct MenuRpcHeader in) {
	pkt_writeUint8(pkt, in.type);
}
struct GameplayRpcHeader pkt_readGameplayRpcHeader(const uint8_t **pkt) {
	struct GameplayRpcHeader out;
	out.type = pkt_readUint8(pkt);
	return out;
}
void pkt_writeGameplayRpcHeader(uint8_t **pkt, struct GameplayRpcHeader in) {
	pkt_writeUint8(pkt, in.type);
}
struct MpBeatmapPacket pkt_readMpBeatmapPacket(const uint8_t **pkt) {
	struct MpBeatmapPacket out;
	out.levelHash = pkt_readString(pkt);
	out.songName = pkt_readLongString(pkt);
	out.songSubName = pkt_readLongString(pkt);
	out.songAuthorName = pkt_readLongString(pkt);
	out.levelAuthorName = pkt_readLongString(pkt);
	out.beatsPerMinute = pkt_readFloat32(pkt);
	out.songDuration = pkt_readFloat32(pkt);
	out.characteristic = pkt_readString(pkt);
	out.difficulty = pkt_readUint32(pkt);
	return out;
}
struct MpPlayerData pkt_readMpPlayerData(const uint8_t **pkt) {
	struct MpPlayerData out;
	out.platformId = pkt_readString(pkt);
	out.platform = pkt_readInt32(pkt);
	return out;
}
struct AuthenticateUserRequest pkt_readAuthenticateUserRequest(const uint8_t **pkt) {
	struct AuthenticateUserRequest out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	out.authenticationToken = pkt_readAuthenticationToken(pkt);
	return out;
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
		pkt_writeServerCode(pkt, in.code);
		pkt_writeGameplayServerConfiguration(pkt, in.configuration);
		pkt_writeString(pkt, in.managerId);
	}
}
struct ConnectToServerRequest pkt_readConnectToServerRequest(const uint8_t **pkt) {
	struct ConnectToServerRequest out;
	out.base = pkt_readBaseConnectToServerRequest(pkt);
	out.selectionMask = pkt_readBeatmapLevelSelectionMask(pkt);
	out.secret = pkt_readString(pkt);
	out.code = pkt_readServerCode(pkt);
	out.configuration = pkt_readGameplayServerConfiguration(pkt);
	return out;
}
struct ClientHelloRequest pkt_readClientHelloRequest(const uint8_t **pkt) {
	struct ClientHelloRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(pkt);
	pkt_readUint8Array(pkt, out.random, 32);
	return out;
}
void pkt_writeHelloVerifyRequest(uint8_t **pkt, struct HelloVerifyRequest in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
	pkt_writeUint8Array(pkt, in.cookie, 32);
}
struct ClientHelloWithCookieRequest pkt_readClientHelloWithCookieRequest(const uint8_t **pkt) {
	struct ClientHelloWithCookieRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(pkt);
	out.certificateResponseId = pkt_readUint32(pkt);
	pkt_readUint8Array(pkt, out.random, 32);
	pkt_readUint8Array(pkt, out.cookie, 32);
	return out;
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
struct ClientKeyExchangeRequest pkt_readClientKeyExchangeRequest(const uint8_t **pkt) {
	struct ClientKeyExchangeRequest out;
	out.base = pkt_readBaseMasterServerReliableResponse(pkt);
	out.clientPublicKey = pkt_readByteArrayNetSerializable(pkt);
	return out;
}
void pkt_writeChangeCipherSpecRequest(uint8_t **pkt, struct ChangeCipherSpecRequest in) {
	pkt_writeBaseMasterServerReliableResponse(pkt, in.base);
}
void pkt_logMessageType(const char *name, char *buf, char *it, MessageType in) {
	fprintf(stderr, "%.*s%s=%u (%s)\n", (uint32_t)(it - buf), buf, name, in, reflect(MessageType, in));
}
struct MessageHeader pkt_readMessageHeader(const uint8_t **pkt) {
	struct MessageHeader out;
	out.type = pkt_readUint32(pkt);
	out.protocolVersion = pkt_readVarUint32(pkt);
	return out;
}
void pkt_writeMessageHeader(uint8_t **pkt, struct MessageHeader in) {
	pkt_writeUint32(pkt, in.type);
	pkt_writeVarUint32(pkt, in.protocolVersion);
}
struct SerializeHeader pkt_readSerializeHeader(const uint8_t **pkt) {
	struct SerializeHeader out;
	out.length = pkt_readVarUint32(pkt);
	out.type = pkt_readUint8(pkt);
	return out;
}
void pkt_writeSerializeHeader(uint8_t **pkt, struct SerializeHeader in) {
	pkt_writeVarUint32(pkt, in.length);
	pkt_writeUint8(pkt, in.type);
}
