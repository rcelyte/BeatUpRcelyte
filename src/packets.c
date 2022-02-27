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
uint8_t pkt_readUint8(struct PacketContext ctx, const uint8_t **pkt) {
	uint8_t v = (*pkt)[0];
	*pkt += sizeof(v);
	return v;
}
uint16_t pkt_readUint16(struct PacketContext ctx, const uint8_t **pkt) {
	uint16_t v = (*pkt)[0] | (*pkt)[1] << 8;
	*pkt += sizeof(v);
	return v;
}
uint32_t pkt_readUint32(struct PacketContext ctx, const uint8_t **pkt) {
	uint32_t v = (*pkt)[0] | (*pkt)[1] << 8 | (*pkt)[2] << 16 | (*pkt)[3] << 24;
	*pkt += sizeof(v);
	return v;
}
uint64_t pkt_readUint64(struct PacketContext ctx, const uint8_t **pkt) {
	uint64_t v = (uint64_t)(*pkt)[0] | (uint64_t)(*pkt)[1] << 8 | (uint64_t)(*pkt)[2] << 16 | (uint64_t)(*pkt)[3] << 24 | (uint64_t)(*pkt)[4] << 32 | (uint64_t)(*pkt)[5] << 40 | (uint64_t)(*pkt)[6] << 48 | (uint64_t)(*pkt)[7] << 56;
	*pkt += sizeof(v);
	return v;
}
/*uint64_t pkt_readVarUint64(struct PacketContext ctx, const uint8_t **pkt) {
	uint64_t byte, value = 0;
	uint32_t shift = 0;
	for(; (byte = (uint64_t)pkt_readUint8(ctx, pkt)) & 128; shift += 7)
		value |= (byte & 127) << shift;
	return value | byte << shift;
}*/
uint64_t pkt_readVarUint64(struct PacketContext ctx, const uint8_t **pkt) {
	uint64_t byte, value = 0;
	uint8_t shift = 0;
	do {
		if(shift >= 64) {
			fprintf(stderr, "Buffer overflow in read of VarUint\n");
			*pkt = _trap;
			return 0;
		}
		byte = pkt_readUint8(ctx, pkt);
		value |= (byte & 127) << shift;
		shift += 7;
	} while(byte & 128);
	return value;
}
int64_t pkt_readVarInt64(struct PacketContext ctx, const uint8_t **pkt) {
	int64_t varULong = (int64_t)pkt_readVarUint64(ctx, pkt);
	if((varULong & 1L) != 1L)
		return varULong >> 1;
	return -(varULong >> 1) + 1L;
}
uint32_t pkt_readVarUint32(struct PacketContext ctx, const uint8_t **pkt) {
	return (uint32_t)pkt_readVarUint64(ctx, pkt);
}
int32_t pkt_readVarInt32(struct PacketContext ctx, const uint8_t **pkt) {
	return (int32_t)pkt_readVarInt64(ctx, pkt);
}
#define pkt_readInt8Array(pkt, out, count) pkt_readUint8Array(pkt, (uint8_t*)out, count)
void pkt_readUint8Array(const uint8_t **pkt, uint8_t *out, uint32_t count) {
	memcpy(out, *pkt, count);
	*pkt += count;
}
float pkt_readFloat32(struct PacketContext ctx, const uint8_t **pkt) {
	float v = *(const float*)*pkt;
	*pkt += 4;
	return v;
}
double pkt_readFloat64(struct PacketContext ctx, const uint8_t **pkt) {
	double v = *(const double*)*pkt;
	*pkt += 8;
	return v;
}
void pkt_writeUint8(struct PacketContext ctx, uint8_t **pkt, uint8_t v) {
	(*pkt)[0] = v;
	*pkt += sizeof(v);
}
void pkt_writeUint16(struct PacketContext ctx, uint8_t **pkt, uint16_t v) {
	(*pkt)[0] = v & 255;
	(*pkt)[1] = v >> 8 & 255;
	*pkt += sizeof(v);
}
void pkt_writeUint32(struct PacketContext ctx, uint8_t **pkt, uint32_t v) {
	(*pkt)[0] = v & 255;
	(*pkt)[1] = v >> 8 & 255;
	(*pkt)[2] = v >> 16 & 255;
	(*pkt)[3] = v >> 24 & 255;
	*pkt += sizeof(v);
}
void pkt_writeUint64(struct PacketContext ctx, uint8_t **pkt, uint64_t v) {
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
void pkt_writeVarUint64(struct PacketContext ctx, uint8_t **pkt, uint64_t v) {
	do {
		uint8_t byte = v & 127;
		v >>= 7;
		if(v)
			byte |= 128;
		pkt_writeUint8(ctx, pkt, byte);
	} while(v);
}
void pkt_writeVarInt64(struct PacketContext ctx, uint8_t **pkt, int64_t v) {
	if(v < 0)
		pkt_writeVarUint64(ctx, pkt, (-(v + 1L) << 1) + 1L);
	else
		pkt_writeVarUint64(ctx, pkt, v << 1);
}
void pkt_writeVarUint32(struct PacketContext ctx, uint8_t **pkt, uint32_t v) {
	pkt_writeVarUint64(ctx, pkt, v);
}
void pkt_writeVarInt32(struct PacketContext ctx, uint8_t **pkt, int32_t v) {
	pkt_writeVarInt64(ctx, pkt, v);
}
void pkt_writeUint8Array(struct PacketContext ctx, uint8_t **pkt, const uint8_t *in, uint32_t count) {
	memcpy(*pkt, in, count);
	*pkt += count;
}
void pkt_writeFloat32(struct PacketContext ctx, uint8_t **pkt, float v) {
	*(float*)*pkt = v;
	*pkt += 4;
}
void pkt_writeFloat64(struct PacketContext ctx, uint8_t **pkt, double v) {
	*(double*)*pkt = v;
	*pkt += 8;
}
#define pkt_logUint8(ctx, name, buf, it, v) pkt_logUint64(ctx, name, buf, it, (uint64_t)v)
#define pkt_logUint16(ctx, name, buf, it, v) pkt_logUint64(ctx, name, buf, it, (uint64_t)v)
#define pkt_logUint32(ctx, name, buf, it, v) pkt_logUint64(ctx, name, buf, it, (uint64_t)v)
#define pkt_logInt32(ctx, name, buf, it, v) pkt_logInt64(ctx, name, buf, it, (int64_t)v)
#define pkt_logVarUint64 pkt_logUint64
#define pkt_logVarInt64 pkt_logInt64
#define pkt_logVarUint32 pkt_logUint32
#define pkt_logVarInt32 pkt_logInt32
static void pkt_logUint64(struct PacketContext ctx, const char *name, char *buf, char *it, uint64_t v) {
	fprintf(stderr, "%.*s%s=%" PRIu64 "\n", (uint32_t)(it - buf), buf, name, v);
}
static void pkt_logInt64(struct PacketContext ctx, const char *name, char *buf, char *it, int64_t v) {
	fprintf(stderr, "%.*s%s=%" PRId64 "\n", (uint32_t)(it - buf), buf, name, v);
}
#define pkt_logInt8Array(ctx, name, buf, it, in, count) pkt_logUint8Array(ctx, name, buf, it, (const uint8_t*)in, count)
static void pkt_logUint8Array(struct PacketContext ctx, const char *name, char *buf, char *it, const uint8_t *in, uint32_t count) {
	fprintf(stderr, "%.*s%s=", (uint32_t)(it - buf), buf, name);
	for(uint32_t i = 0; i < count; ++i)
		fprintf(stderr, "%02hhx", in[i]);
	fprintf(stderr, "\n");
}
#define pkt_logFloat32 pkt_logFloat64
static void pkt_logFloat64(struct PacketContext ctx, const char *name, char *buf, char *it, double v) {
	fprintf(stderr, "%.*s%s=%f\n", (uint32_t)(it - buf), buf, name, v);
}
struct PacketEncryptionLayer pkt_readPacketEncryptionLayer(struct PacketContext ctx, const uint8_t **pkt) {
	struct PacketEncryptionLayer out;
	out.encrypted = pkt_readUint8(ctx, pkt);
	if(out.encrypted == 1) {
		out.sequenceId = pkt_readUint32(ctx, pkt);
		pkt_readUint8Array(pkt, out.iv, 16);
	}
	return out;
}
void pkt_writePacketEncryptionLayer(struct PacketContext ctx, uint8_t **pkt, struct PacketEncryptionLayer in) {
	pkt_writeUint8(ctx, pkt, in.encrypted);
	if(in.encrypted == 1) {
		pkt_writeUint32(ctx, pkt, in.sequenceId);
		pkt_writeUint8Array(ctx, pkt, in.iv, 16);
	}
}
struct BaseMasterServerReliableRequest pkt_readBaseMasterServerReliableRequest(struct PacketContext ctx, const uint8_t **pkt) {
	struct BaseMasterServerReliableRequest out;
	out.requestId = pkt_readUint32(ctx, pkt);
	return out;
}
void pkt_writeBaseMasterServerReliableRequest(struct PacketContext ctx, uint8_t **pkt, struct BaseMasterServerReliableRequest in) {
	pkt_writeUint32(ctx, pkt, in.requestId);
}
struct BaseMasterServerResponse pkt_readBaseMasterServerResponse(struct PacketContext ctx, const uint8_t **pkt) {
	struct BaseMasterServerResponse out;
	out.responseId = pkt_readUint32(ctx, pkt);
	return out;
}
void pkt_writeBaseMasterServerResponse(struct PacketContext ctx, uint8_t **pkt, struct BaseMasterServerResponse in) {
	pkt_writeUint32(ctx, pkt, in.responseId);
}
struct BaseMasterServerReliableResponse pkt_readBaseMasterServerReliableResponse(struct PacketContext ctx, const uint8_t **pkt) {
	struct BaseMasterServerReliableResponse out;
	out.requestId = pkt_readUint32(ctx, pkt);
	out.responseId = pkt_readUint32(ctx, pkt);
	return out;
}
void pkt_writeBaseMasterServerReliableResponse(struct PacketContext ctx, uint8_t **pkt, struct BaseMasterServerReliableResponse in) {
	pkt_writeUint32(ctx, pkt, in.requestId);
	pkt_writeUint32(ctx, pkt, in.responseId);
}
struct BaseMasterServerAcknowledgeMessage pkt_readBaseMasterServerAcknowledgeMessage(struct PacketContext ctx, const uint8_t **pkt) {
	struct BaseMasterServerAcknowledgeMessage out;
	out.base = pkt_readBaseMasterServerResponse(ctx, pkt);
	out.messageHandled = pkt_readUint8(ctx, pkt);
	return out;
}
void pkt_writeBaseMasterServerAcknowledgeMessage(struct PacketContext ctx, uint8_t **pkt, struct BaseMasterServerAcknowledgeMessage in) {
	pkt_writeBaseMasterServerResponse(ctx, pkt, in.base);
	pkt_writeUint8(ctx, pkt, in.messageHandled);
}
struct ByteArrayNetSerializable pkt_readByteArrayNetSerializable(struct PacketContext ctx, const uint8_t **pkt) {
	struct ByteArrayNetSerializable out;
	out.length = pkt_readVarUint32(ctx, pkt);
	if(out.length > 4096) {
		fprintf(stderr, "Buffer overflow in read of ByteArrayNetSerializable.data: %u > 4096\n", (uint32_t)out.length), out.length = 0, *pkt = _trap;
	} else {
		pkt_readUint8Array(pkt, out.data, out.length);
	}
	return out;
}
void pkt_writeByteArrayNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct ByteArrayNetSerializable in) {
	pkt_writeVarUint32(ctx, pkt, in.length);
	pkt_writeUint8Array(ctx, pkt, in.data, in.length);
}
struct String pkt_readString(struct PacketContext ctx, const uint8_t **pkt) {
	struct String out;
	if(ctx.netVersion >= 12) {
		out.length = pkt_readUint16(ctx, pkt);
		out.isNull = (out.length != 0);
		if(out.length)
			--out.length;
	} else {
		out.length = pkt_readUint32(ctx, pkt);
		out.isNull = 0;
	}
	if(out.length > 60) {
		fprintf(stderr, "Buffer overflow in read of String.data: %u > 60\n", (uint32_t)out.length), out.length = 0, *pkt = _trap;
	} else {
		pkt_readInt8Array(pkt, out.data, out.length);
	}
	return out;
}
struct LongString pkt_readLongString(struct PacketContext ctx, const uint8_t **pkt) {
	struct LongString out;
	if(ctx.netVersion >= 12) {
		out.length = pkt_readUint16(ctx, pkt);
		out.isNull = (out.length != 0);
		if(out.length)
			--out.length;
	} else {
		out.length = pkt_readUint32(ctx, pkt);
		out.isNull = 0;
	}
	if(out.length > 4096) {
		fprintf(stderr, "Buffer overflow in read of LongString.data: %u > 4096\n", (uint32_t)out.length), out.length = 0, *pkt = _trap;
	} else {
		pkt_readInt8Array(pkt, out.data, out.length);
	}
	return out;
}
void pkt_writeString(struct PacketContext ctx, uint8_t **pkt, struct String in) {
	if(ctx.netVersion >= 12)
		pkt_writeUint16(ctx, pkt, in.length + !in.isNull);
	else
		pkt_writeUint32(ctx, pkt, in.length);
	pkt_writeInt8Array(ctx, pkt, in.data, in.length);
}
void pkt_writeLongString(struct PacketContext ctx, uint8_t **pkt, struct LongString in) {
	if(ctx.netVersion >= 12)
		pkt_writeUint16(ctx, pkt, in.length + !in.isNull);
	else
		pkt_writeUint32(ctx, pkt, in.length);
	pkt_writeInt8Array(ctx, pkt, in.data, in.length);
}
#ifdef PACKET_LOGGING_FUNCS
static void pkt_logString(struct PacketContext ctx, const char *name, char *buf, char *it, struct String in) {
	fprintf(stderr, "%.*s%s=\"%.*s\"\n", (uint32_t)(it - buf), buf, name, in.length, in.data);
}
static void pkt_logLongString(struct PacketContext ctx, const char *name, char *buf, char *it, struct LongString in) {
	fprintf(stderr, "%.*s%s=\"%.*s\"\n", (uint32_t)(it - buf), buf, name, in.length, in.data);
}
#endif
struct AuthenticationToken pkt_readAuthenticationToken(struct PacketContext ctx, const uint8_t **pkt) {
	struct AuthenticationToken out;
	out.platform = pkt_readUint8(ctx, pkt);
	out.userId = pkt_readString(ctx, pkt);
	out.userName = pkt_readString(ctx, pkt);
	out.sessionToken = pkt_readByteArrayNetSerializable(ctx, pkt);
	return out;
}
struct BaseMasterServerMultipartMessage pkt_readBaseMasterServerMultipartMessage(struct PacketContext ctx, const uint8_t **pkt) {
	struct BaseMasterServerMultipartMessage out;
	out.base = pkt_readBaseMasterServerReliableRequest(ctx, pkt);
	out.multipartMessageId = pkt_readUint32(ctx, pkt);
	out.offset = pkt_readVarUint32(ctx, pkt);
	out.length = pkt_readVarUint32(ctx, pkt);
	out.totalLength = pkt_readVarUint32(ctx, pkt);
	if(out.length > 384) {
		fprintf(stderr, "Buffer overflow in read of BaseMasterServerMultipartMessage.data: %u > 384\n", (uint32_t)out.length), out.length = 0, *pkt = _trap;
	} else {
		pkt_readUint8Array(pkt, out.data, out.length);
	}
	return out;
}
void pkt_writeBaseMasterServerMultipartMessage(struct PacketContext ctx, uint8_t **pkt, struct BaseMasterServerMultipartMessage in) {
	pkt_writeBaseMasterServerReliableRequest(ctx, pkt, in.base);
	pkt_writeUint32(ctx, pkt, in.multipartMessageId);
	pkt_writeVarUint32(ctx, pkt, in.offset);
	pkt_writeVarUint32(ctx, pkt, in.length);
	pkt_writeVarUint32(ctx, pkt, in.totalLength);
	pkt_writeUint8Array(ctx, pkt, in.data, in.length);
}
struct BitMask128 pkt_readBitMask128(struct PacketContext ctx, const uint8_t **pkt) {
	struct BitMask128 out;
	out.d0 = pkt_readUint64(ctx, pkt);
	out.d1 = pkt_readUint64(ctx, pkt);
	return out;
}
void pkt_writeBitMask128(struct PacketContext ctx, uint8_t **pkt, struct BitMask128 in) {
	pkt_writeUint64(ctx, pkt, in.d0);
	pkt_writeUint64(ctx, pkt, in.d1);
}
struct SongPackMask pkt_readSongPackMask(struct PacketContext ctx, const uint8_t **pkt) {
	struct SongPackMask out;
	out.bloomFilter = pkt_readBitMask128(ctx, pkt);
	return out;
}
void pkt_writeSongPackMask(struct PacketContext ctx, uint8_t **pkt, struct SongPackMask in) {
	pkt_writeBitMask128(ctx, pkt, in.bloomFilter);
}
struct BeatmapLevelSelectionMask pkt_readBeatmapLevelSelectionMask(struct PacketContext ctx, const uint8_t **pkt) {
	struct BeatmapLevelSelectionMask out;
	out.difficulties = pkt_readUint8(ctx, pkt);
	out.modifiers = pkt_readUint32(ctx, pkt);
	out.songPacks = pkt_readSongPackMask(ctx, pkt);
	return out;
}
void pkt_writeBeatmapLevelSelectionMask(struct PacketContext ctx, uint8_t **pkt, struct BeatmapLevelSelectionMask in) {
	pkt_writeUint8(ctx, pkt, in.difficulties);
	pkt_writeUint32(ctx, pkt, in.modifiers);
	pkt_writeSongPackMask(ctx, pkt, in.songPacks);
}
struct GameplayServerConfiguration pkt_readGameplayServerConfiguration(struct PacketContext ctx, const uint8_t **pkt) {
	struct GameplayServerConfiguration out;
	out.maxPlayerCount = pkt_readVarInt32(ctx, pkt);
	out.discoveryPolicy = pkt_readVarInt32(ctx, pkt);
	out.invitePolicy = pkt_readVarInt32(ctx, pkt);
	out.gameplayServerMode = pkt_readVarInt32(ctx, pkt);
	out.songSelectionMode = pkt_readVarInt32(ctx, pkt);
	out.gameplayServerControlSettings = pkt_readVarInt32(ctx, pkt);
	return out;
}
void pkt_writeGameplayServerConfiguration(struct PacketContext ctx, uint8_t **pkt, struct GameplayServerConfiguration in) {
	pkt_writeVarInt32(ctx, pkt, in.maxPlayerCount);
	pkt_writeVarInt32(ctx, pkt, in.discoveryPolicy);
	pkt_writeVarInt32(ctx, pkt, in.invitePolicy);
	pkt_writeVarInt32(ctx, pkt, in.gameplayServerMode);
	pkt_writeVarInt32(ctx, pkt, in.songSelectionMode);
	pkt_writeVarInt32(ctx, pkt, in.gameplayServerControlSettings);
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
static ServerCode pkt_readServerCode(struct PacketContext ctx, const uint8_t **pkt) {
	struct String str = pkt_readString(ctx, pkt);
	return StringToServerCode(str.data, str.length);
}
static void pkt_writeServerCode(struct PacketContext ctx, uint8_t **pkt, ServerCode in) {
	struct String str = {.length = 0};
	for(in = scramble_encode(in); in; in /= 36)
		str.data[str.length++] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[--in % 36];
	pkt_writeString(ctx, pkt, str);
}
#ifdef PACKET_LOGGING_FUNCS
static void pkt_logServerCode(struct PacketContext ctx, const char *name, char *buf, char *it, ServerCode in) {
	fprintf(stderr, "%.*s%s=%u (\"", (uint32_t)(it - buf), buf, name, in);
	for(; in; in /= 36)
		fprintf(stderr, "%c", "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[--in % 36]);
	fprintf(stderr, "\")\n");
}
#endif
void pkt_writeIPEndPoint(struct PacketContext ctx, uint8_t **pkt, struct IPEndPoint in) {
	pkt_writeString(ctx, pkt, in.address);
	pkt_writeUint32(ctx, pkt, in.port);
}
struct BaseConnectToServerRequest pkt_readBaseConnectToServerRequest(struct PacketContext ctx, const uint8_t **pkt) {
	struct BaseConnectToServerRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(ctx, pkt);
	out.userId = pkt_readString(ctx, pkt);
	out.userName = pkt_readString(ctx, pkt);
	pkt_readUint8Array(pkt, out.random, 32);
	out.publicKey = pkt_readByteArrayNetSerializable(ctx, pkt);
	return out;
}
struct Channeled pkt_readChanneled(struct PacketContext ctx, const uint8_t **pkt) {
	struct Channeled out;
	out.sequence = pkt_readUint16(ctx, pkt);
	out.channelId = pkt_readUint8(ctx, pkt);
	return out;
}
void pkt_writeChanneled(struct PacketContext ctx, uint8_t **pkt, struct Channeled in) {
	pkt_writeUint16(ctx, pkt, in.sequence);
	pkt_writeUint8(ctx, pkt, in.channelId);
}
struct Ack pkt_readAck(struct PacketContext ctx, const uint8_t **pkt) {
	struct Ack out;
	out.sequence = pkt_readUint16(ctx, pkt);
	out.channelId = pkt_readUint8(ctx, pkt);
	if(out.channelId % 2 == 0) {
		pkt_readUint8Array(pkt, out.data, 8);
		out._pad0 = pkt_readUint8(ctx, pkt);
	}
	return out;
}
void pkt_writeAck(struct PacketContext ctx, uint8_t **pkt, struct Ack in) {
	pkt_writeUint16(ctx, pkt, in.sequence);
	pkt_writeUint8(ctx, pkt, in.channelId);
	if(in.channelId % 2 == 0) {
		pkt_writeUint8Array(ctx, pkt, in.data, 8);
		pkt_writeUint8(ctx, pkt, in._pad0);
	}
}
struct Ping pkt_readPing(struct PacketContext ctx, const uint8_t **pkt) {
	struct Ping out;
	out.sequence = pkt_readUint16(ctx, pkt);
	return out;
}
void pkt_writePing(struct PacketContext ctx, uint8_t **pkt, struct Ping in) {
	pkt_writeUint16(ctx, pkt, in.sequence);
}
struct Pong pkt_readPong(struct PacketContext ctx, const uint8_t **pkt) {
	struct Pong out;
	out.sequence = pkt_readUint16(ctx, pkt);
	out.time = pkt_readUint64(ctx, pkt);
	return out;
}
void pkt_writePong(struct PacketContext ctx, uint8_t **pkt, struct Pong in) {
	pkt_writeUint16(ctx, pkt, in.sequence);
	pkt_writeUint64(ctx, pkt, in.time);
}
struct ConnectRequest pkt_readConnectRequest(struct PacketContext ctx, const uint8_t **pkt) {
	struct ConnectRequest out;
	out.protocolId = pkt_readUint32(ctx, pkt);
	out.connectTime = pkt_readUint64(ctx, pkt);
	if((ctx.netVersion = out.protocolId) >= 12) {
		out.peerId = pkt_readInt32(ctx, pkt);
	}
	out.addrlen = pkt_readUint8(ctx, pkt);
	if(out.addrlen > 38) {
		fprintf(stderr, "Buffer overflow in read of ConnectRequest.address: %u > 38\n", (uint32_t)out.addrlen), out.addrlen = 0, *pkt = _trap;
	} else {
		pkt_readUint8Array(pkt, out.address, out.addrlen);
	}
	out.secret = pkt_readString(ctx, pkt);
	out.userId = pkt_readString(ctx, pkt);
	out.userName = pkt_readString(ctx, pkt);
	out.isConnectionOwner = pkt_readUint8(ctx, pkt);
	return out;
}
void pkt_writeConnectAccept(struct PacketContext ctx, uint8_t **pkt, struct ConnectAccept in) {
	pkt_writeUint64(ctx, pkt, in.connectTime);
	pkt_writeUint8(ctx, pkt, in.connectNum);
	pkt_writeUint8(ctx, pkt, in.reusedPeer);
	if(ctx.netVersion >= 12) {
		pkt_writeInt32(ctx, pkt, in.peerId);
	}
}
struct Disconnect pkt_readDisconnect(struct PacketContext ctx, const uint8_t **pkt) {
	struct Disconnect out;
	pkt_readUint8Array(pkt, out._pad0, 8);
	return out;
}
struct MtuCheck pkt_readMtuCheck(struct PacketContext ctx, const uint8_t **pkt) {
	struct MtuCheck out;
	out.newMtu0 = pkt_readUint32(ctx, pkt);
	if(out.newMtu0-9 > 1423) {
		fprintf(stderr, "Buffer overflow in read of MtuCheck.pad: %u > 1423\n", (uint32_t)out.newMtu0-9), out.newMtu0 = 0, *pkt = _trap;
	} else {
		pkt_readUint8Array(pkt, out.pad, out.newMtu0-9);
	}
	out.newMtu1 = pkt_readUint32(ctx, pkt);
	return out;
}
void pkt_writeMtuOk(struct PacketContext ctx, uint8_t **pkt, struct MtuOk in) {
	pkt_writeUint32(ctx, pkt, in.newMtu0);
	pkt_writeUint8Array(ctx, pkt, in.pad, in.newMtu0-9);
	pkt_writeUint32(ctx, pkt, in.newMtu1);
}
struct NetPacketHeader pkt_readNetPacketHeader(struct PacketContext ctx, const uint8_t **pkt) {
	struct NetPacketHeader out;
	uint8_t bits = pkt_readUint8(ctx, pkt);
	out.property = (bits >> 0) & 31;
	out.connectionNumber = (bits >> 5) & 3;
	out.isFragmented = (bits >> 7) & 1;
	return out;
}
void pkt_writeNetPacketHeader(struct PacketContext ctx, uint8_t **pkt, struct NetPacketHeader in) {
	uint8_t bits = 0;
	bits |= (in.property << 0);
	bits |= (in.connectionNumber << 5);
	bits |= (in.isFragmented << 7);
	pkt_writeUint8(ctx, pkt, bits);
}
struct FragmentedHeader pkt_readFragmentedHeader(struct PacketContext ctx, const uint8_t **pkt) {
	struct FragmentedHeader out;
	out.fragmentId = pkt_readUint16(ctx, pkt);
	out.fragmentPart = pkt_readUint16(ctx, pkt);
	out.fragmentsTotal = pkt_readUint16(ctx, pkt);
	return out;
}
struct PlayerStateHash pkt_readPlayerStateHash(struct PacketContext ctx, const uint8_t **pkt) {
	struct PlayerStateHash out;
	out.bloomFilter = pkt_readBitMask128(ctx, pkt);
	return out;
}
void pkt_writePlayerStateHash(struct PacketContext ctx, uint8_t **pkt, struct PlayerStateHash in) {
	pkt_writeBitMask128(ctx, pkt, in.bloomFilter);
}
struct Color32 pkt_readColor32(struct PacketContext ctx, const uint8_t **pkt) {
	struct Color32 out;
	out.r = pkt_readUint8(ctx, pkt);
	out.g = pkt_readUint8(ctx, pkt);
	out.b = pkt_readUint8(ctx, pkt);
	out.a = pkt_readUint8(ctx, pkt);
	return out;
}
void pkt_writeColor32(struct PacketContext ctx, uint8_t **pkt, struct Color32 in) {
	pkt_writeUint8(ctx, pkt, in.r);
	pkt_writeUint8(ctx, pkt, in.g);
	pkt_writeUint8(ctx, pkt, in.b);
	pkt_writeUint8(ctx, pkt, in.a);
}
struct MultiplayerAvatarData pkt_readMultiplayerAvatarData(struct PacketContext ctx, const uint8_t **pkt) {
	struct MultiplayerAvatarData out;
	out.headTopId = pkt_readString(ctx, pkt);
	out.headTopPrimaryColor = pkt_readColor32(ctx, pkt);
	out.handsColor = pkt_readColor32(ctx, pkt);
	out.clothesId = pkt_readString(ctx, pkt);
	out.clothesPrimaryColor = pkt_readColor32(ctx, pkt);
	out.clothesSecondaryColor = pkt_readColor32(ctx, pkt);
	out.clothesDetailColor = pkt_readColor32(ctx, pkt);
	for(uint32_t i = 0; i < 2; ++i)
		out._unused[i] = pkt_readColor32(ctx, pkt);
	out.eyesId = pkt_readString(ctx, pkt);
	out.mouthId = pkt_readString(ctx, pkt);
	out.glassesColor = pkt_readColor32(ctx, pkt);
	out.facialHairColor = pkt_readColor32(ctx, pkt);
	out.headTopSecondaryColor = pkt_readColor32(ctx, pkt);
	out.glassesId = pkt_readString(ctx, pkt);
	out.facialHairId = pkt_readString(ctx, pkt);
	out.handsId = pkt_readString(ctx, pkt);
	return out;
}
void pkt_writeMultiplayerAvatarData(struct PacketContext ctx, uint8_t **pkt, struct MultiplayerAvatarData in) {
	pkt_writeString(ctx, pkt, in.headTopId);
	pkt_writeColor32(ctx, pkt, in.headTopPrimaryColor);
	pkt_writeColor32(ctx, pkt, in.handsColor);
	pkt_writeString(ctx, pkt, in.clothesId);
	pkt_writeColor32(ctx, pkt, in.clothesPrimaryColor);
	pkt_writeColor32(ctx, pkt, in.clothesSecondaryColor);
	pkt_writeColor32(ctx, pkt, in.clothesDetailColor);
	for(uint32_t i = 0; i < 2; ++i)
		pkt_writeColor32(ctx, pkt, in._unused[i]);
	pkt_writeString(ctx, pkt, in.eyesId);
	pkt_writeString(ctx, pkt, in.mouthId);
	pkt_writeColor32(ctx, pkt, in.glassesColor);
	pkt_writeColor32(ctx, pkt, in.facialHairColor);
	pkt_writeColor32(ctx, pkt, in.headTopSecondaryColor);
	pkt_writeString(ctx, pkt, in.glassesId);
	pkt_writeString(ctx, pkt, in.facialHairId);
	pkt_writeString(ctx, pkt, in.handsId);
}
struct RoutingHeader pkt_readRoutingHeader(struct PacketContext ctx, const uint8_t **pkt) {
	struct RoutingHeader out;
	out.remoteConnectionId = pkt_readUint8(ctx, pkt);
	uint8_t bits = pkt_readUint8(ctx, pkt);
	out.connectionId = (bits >> 0) & 127;
	out.encrypted = (bits >> 7) & 1;
	return out;
}
void pkt_writeRoutingHeader(struct PacketContext ctx, uint8_t **pkt, struct RoutingHeader in) {
	pkt_writeUint8(ctx, pkt, in.remoteConnectionId);
	uint8_t bits = 0;
	bits |= (in.connectionId << 0);
	bits |= (in.encrypted << 7);
	pkt_writeUint8(ctx, pkt, bits);
}
void pkt_writeSyncTime(struct PacketContext ctx, uint8_t **pkt, struct SyncTime in) {
	pkt_writeFloat32(ctx, pkt, in.syncTime);
}
void pkt_writePlayerConnected(struct PacketContext ctx, uint8_t **pkt, struct PlayerConnected in) {
	pkt_writeUint8(ctx, pkt, in.remoteConnectionId);
	pkt_writeString(ctx, pkt, in.userId);
	pkt_writeString(ctx, pkt, in.userName);
	pkt_writeUint8(ctx, pkt, in.isConnectionOwner);
}
struct PlayerIdentity pkt_readPlayerIdentity(struct PacketContext ctx, const uint8_t **pkt) {
	struct PlayerIdentity out;
	out.playerState = pkt_readPlayerStateHash(ctx, pkt);
	out.playerAvatar = pkt_readMultiplayerAvatarData(ctx, pkt);
	out.random = pkt_readByteArrayNetSerializable(ctx, pkt);
	out.publicEncryptionKey = pkt_readByteArrayNetSerializable(ctx, pkt);
	return out;
}
void pkt_writePlayerIdentity(struct PacketContext ctx, uint8_t **pkt, struct PlayerIdentity in) {
	pkt_writePlayerStateHash(ctx, pkt, in.playerState);
	pkt_writeMultiplayerAvatarData(ctx, pkt, in.playerAvatar);
	pkt_writeByteArrayNetSerializable(ctx, pkt, in.random);
	pkt_writeByteArrayNetSerializable(ctx, pkt, in.publicEncryptionKey);
}
void pkt_writePlayerLatencyUpdate(struct PacketContext ctx, uint8_t **pkt, struct PlayerLatencyUpdate in) {
	pkt_writeFloat32(ctx, pkt, in.latency);
}
void pkt_writePlayerDisconnected(struct PacketContext ctx, uint8_t **pkt, struct PlayerDisconnected in) {
	pkt_writeVarInt32(ctx, pkt, in.disconnectedReason);
}
void pkt_writePlayerSortOrderUpdate(struct PacketContext ctx, uint8_t **pkt, struct PlayerSortOrderUpdate in) {
	pkt_writeString(ctx, pkt, in.userId);
	pkt_writeVarInt32(ctx, pkt, in.sortIndex);
}
struct PlayerStateUpdate pkt_readPlayerStateUpdate(struct PacketContext ctx, const uint8_t **pkt) {
	struct PlayerStateUpdate out;
	out.playerState = pkt_readPlayerStateHash(ctx, pkt);
	return out;
}
struct PingMessage pkt_readPingMessage(struct PacketContext ctx, const uint8_t **pkt) {
	struct PingMessage out;
	out.pingTime = pkt_readFloat32(ctx, pkt);
	return out;
}
struct PongMessage pkt_readPongMessage(struct PacketContext ctx, const uint8_t **pkt) {
	struct PongMessage out;
	out.pingTime = pkt_readFloat32(ctx, pkt);
	return out;
}
void pkt_writePongMessage(struct PacketContext ctx, uint8_t **pkt, struct PongMessage in) {
	pkt_writeFloat32(ctx, pkt, in.pingTime);
}
struct RemoteProcedureCall pkt_readRemoteProcedureCall(struct PacketContext ctx, const uint8_t **pkt) {
	struct RemoteProcedureCall out;
	out.syncTime = pkt_readFloat32(ctx, pkt);
	return out;
}
void pkt_writeRemoteProcedureCall(struct PacketContext ctx, uint8_t **pkt, struct RemoteProcedureCall in) {
	pkt_writeFloat32(ctx, pkt, in.syncTime);
}
struct RemoteProcedureCallFlags pkt_readRemoteProcedureCallFlags(struct PacketContext ctx, const uint8_t **pkt) {
	struct RemoteProcedureCallFlags out;
	uint8_t bits = 255;
	if(ctx.protocolVersion > 6)
		bits = pkt_readUint8(ctx, pkt);
	out.hasValue0 = (bits >> 0) & 1;
	out.hasValue1 = (bits >> 1) & 1;
	out.hasValue2 = (bits >> 2) & 1;
	out.hasValue3 = (bits >> 3) & 1;
	return out;
}
void pkt_writeRemoteProcedureCallFlags(struct PacketContext ctx, uint8_t **pkt, struct RemoteProcedureCallFlags in) {
	if(ctx.protocolVersion > 6) {
		uint8_t bits = 0;
		bits |= (in.hasValue0 << 0);
		bits |= (in.hasValue1 << 1);
		bits |= (in.hasValue2 << 2);
		bits |= (in.hasValue3 << 3);
		pkt_writeUint8(ctx, pkt, bits);
	}
}
void pkt_writePlayersMissingEntitlementsNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct PlayersMissingEntitlementsNetSerializable in) {
	pkt_writeInt32(ctx, pkt, in.count);
	for(uint32_t i = 0; i < in.count; ++i)
		pkt_writeString(ctx, pkt, in.playersWithoutEntitlements[i]);
}
struct BeatmapIdentifierNetSerializable pkt_readBeatmapIdentifierNetSerializable(struct PacketContext ctx, const uint8_t **pkt) {
	struct BeatmapIdentifierNetSerializable out;
	out.levelID = pkt_readLongString(ctx, pkt);
	out.beatmapCharacteristicSerializedName = pkt_readString(ctx, pkt);
	out.difficulty = pkt_readVarUint32(ctx, pkt);
	return out;
}
void pkt_writeBeatmapIdentifierNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct BeatmapIdentifierNetSerializable in) {
	pkt_writeLongString(ctx, pkt, in.levelID);
	pkt_writeString(ctx, pkt, in.beatmapCharacteristicSerializedName);
	pkt_writeVarUint32(ctx, pkt, in.difficulty);
}
struct GameplayModifiers pkt_readGameplayModifiers(struct PacketContext ctx, const uint8_t **pkt) {
	struct GameplayModifiers out;
	int32_t bits = pkt_readInt32(ctx, pkt);
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
void pkt_writeGameplayModifiers(struct PacketContext ctx, uint8_t **pkt, struct GameplayModifiers in) {
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
	pkt_writeInt32(ctx, pkt, bits);
}
void pkt_writePlayerLobbyPermissionConfigurationNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct PlayerLobbyPermissionConfigurationNetSerializable in) {
	pkt_writeString(ctx, pkt, in.userId);
	uint8_t bits = 0;
	bits |= (in.isServerOwner << 0);
	bits |= (in.hasRecommendBeatmapsPermission << 1);
	bits |= (in.hasRecommendGameplayModifiersPermission << 2);
	bits |= (in.hasKickVotePermission << 3);
	bits |= (in.hasInvitePermission << 4);
	pkt_writeUint8(ctx, pkt, bits);
}
void pkt_writePlayersLobbyPermissionConfigurationNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct PlayersLobbyPermissionConfigurationNetSerializable in) {
	pkt_writeInt32(ctx, pkt, in.count);
	for(uint32_t i = 0; i < in.count; ++i)
		pkt_writePlayerLobbyPermissionConfigurationNetSerializable(ctx, pkt, in.playersPermission[i]);
}
struct SyncStateId pkt_readSyncStateId(struct PacketContext ctx, const uint8_t **pkt) {
	struct SyncStateId out;
	uint8_t bits = pkt_readUint8(ctx, pkt);
	out.id = (bits >> 0) & 127;
	out.same = (bits >> 7) & 1;
	return out;
}
struct Vector3Serializable pkt_readVector3Serializable(struct PacketContext ctx, const uint8_t **pkt) {
	struct Vector3Serializable out;
	out.x = pkt_readVarInt32(ctx, pkt);
	out.y = pkt_readVarInt32(ctx, pkt);
	out.z = pkt_readVarInt32(ctx, pkt);
	return out;
}
struct QuaternionSerializable pkt_readQuaternionSerializable(struct PacketContext ctx, const uint8_t **pkt) {
	struct QuaternionSerializable out;
	out.a = pkt_readVarInt32(ctx, pkt);
	out.b = pkt_readVarInt32(ctx, pkt);
	out.c = pkt_readVarInt32(ctx, pkt);
	return out;
}
struct PoseSerializable pkt_readPoseSerializable(struct PacketContext ctx, const uint8_t **pkt) {
	struct PoseSerializable out;
	out.position = pkt_readVector3Serializable(ctx, pkt);
	out.rotation = pkt_readQuaternionSerializable(ctx, pkt);
	return out;
}
struct ColorNoAlphaSerializable pkt_readColorNoAlphaSerializable(struct PacketContext ctx, const uint8_t **pkt) {
	struct ColorNoAlphaSerializable out;
	out.r = pkt_readFloat32(ctx, pkt);
	out.g = pkt_readFloat32(ctx, pkt);
	out.b = pkt_readFloat32(ctx, pkt);
	return out;
}
void pkt_writeColorNoAlphaSerializable(struct PacketContext ctx, uint8_t **pkt, struct ColorNoAlphaSerializable in) {
	pkt_writeFloat32(ctx, pkt, in.r);
	pkt_writeFloat32(ctx, pkt, in.g);
	pkt_writeFloat32(ctx, pkt, in.b);
}
struct ColorSchemeNetSerializable pkt_readColorSchemeNetSerializable(struct PacketContext ctx, const uint8_t **pkt) {
	struct ColorSchemeNetSerializable out;
	out.saberAColor = pkt_readColorNoAlphaSerializable(ctx, pkt);
	out.saberBColor = pkt_readColorNoAlphaSerializable(ctx, pkt);
	out.obstaclesColor = pkt_readColorNoAlphaSerializable(ctx, pkt);
	out.environmentColor0 = pkt_readColorNoAlphaSerializable(ctx, pkt);
	out.environmentColor1 = pkt_readColorNoAlphaSerializable(ctx, pkt);
	out.environmentColor0Boost = pkt_readColorNoAlphaSerializable(ctx, pkt);
	out.environmentColor1Boost = pkt_readColorNoAlphaSerializable(ctx, pkt);
	return out;
}
void pkt_writeColorSchemeNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct ColorSchemeNetSerializable in) {
	pkt_writeColorNoAlphaSerializable(ctx, pkt, in.saberAColor);
	pkt_writeColorNoAlphaSerializable(ctx, pkt, in.saberBColor);
	pkt_writeColorNoAlphaSerializable(ctx, pkt, in.obstaclesColor);
	pkt_writeColorNoAlphaSerializable(ctx, pkt, in.environmentColor0);
	pkt_writeColorNoAlphaSerializable(ctx, pkt, in.environmentColor1);
	pkt_writeColorNoAlphaSerializable(ctx, pkt, in.environmentColor0Boost);
	pkt_writeColorNoAlphaSerializable(ctx, pkt, in.environmentColor1Boost);
}
struct PlayerSpecificSettingsNetSerializable pkt_readPlayerSpecificSettingsNetSerializable(struct PacketContext ctx, const uint8_t **pkt) {
	struct PlayerSpecificSettingsNetSerializable out;
	out.userId = pkt_readString(ctx, pkt);
	out.userName = pkt_readString(ctx, pkt);
	out.leftHanded = pkt_readUint8(ctx, pkt);
	out.automaticPlayerHeight = pkt_readUint8(ctx, pkt);
	out.playerHeight = pkt_readFloat32(ctx, pkt);
	out.headPosToPlayerHeightOffset = pkt_readFloat32(ctx, pkt);
	out.colorScheme = pkt_readColorSchemeNetSerializable(ctx, pkt);
	return out;
}
void pkt_writePlayerSpecificSettingsNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct PlayerSpecificSettingsNetSerializable in) {
	pkt_writeString(ctx, pkt, in.userId);
	pkt_writeString(ctx, pkt, in.userName);
	pkt_writeUint8(ctx, pkt, in.leftHanded);
	pkt_writeUint8(ctx, pkt, in.automaticPlayerHeight);
	pkt_writeFloat32(ctx, pkt, in.playerHeight);
	pkt_writeFloat32(ctx, pkt, in.headPosToPlayerHeightOffset);
	pkt_writeColorSchemeNetSerializable(ctx, pkt, in.colorScheme);
}
void pkt_writePlayerSpecificSettingsAtStartNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct PlayerSpecificSettingsAtStartNetSerializable in) {
	pkt_writeInt32(ctx, pkt, in.count);
	for(uint32_t i = 0; i < in.count; ++i)
		pkt_writePlayerSpecificSettingsNetSerializable(ctx, pkt, in.activePlayerSpecificSettingsAtGameStart[i]);
}
struct NoteCutInfoNetSerializable pkt_readNoteCutInfoNetSerializable(struct PacketContext ctx, const uint8_t **pkt) {
	struct NoteCutInfoNetSerializable out;
	uint8_t bits = pkt_readUint8(ctx, pkt);
	out.cutWasOk = (bits >> 0) & 1;
	out.saberSpeed = pkt_readFloat32(ctx, pkt);
	out.saberDir = pkt_readVector3Serializable(ctx, pkt);
	out.cutPoint = pkt_readVector3Serializable(ctx, pkt);
	out.cutNormal = pkt_readVector3Serializable(ctx, pkt);
	out.notePosition = pkt_readVector3Serializable(ctx, pkt);
	out.noteScale = pkt_readVector3Serializable(ctx, pkt);
	out.noteRotation = pkt_readQuaternionSerializable(ctx, pkt);
	out.colorType = pkt_readVarInt32(ctx, pkt);
	out.noteLineLayer = pkt_readVarInt32(ctx, pkt);
	out.noteLineIndex = pkt_readVarInt32(ctx, pkt);
	out.noteTime = pkt_readFloat32(ctx, pkt);
	out.timeToNextColorNote = pkt_readFloat32(ctx, pkt);
	out.moveVec = pkt_readVector3Serializable(ctx, pkt);
	return out;
}
struct NoteMissInfoNetSerializable pkt_readNoteMissInfoNetSerializable(struct PacketContext ctx, const uint8_t **pkt) {
	struct NoteMissInfoNetSerializable out;
	out.colorType = pkt_readVarInt32(ctx, pkt);
	out.noteLineLayer = pkt_readVarInt32(ctx, pkt);
	out.noteLineIndex = pkt_readVarInt32(ctx, pkt);
	out.noteTime = pkt_readFloat32(ctx, pkt);
	return out;
}
struct LevelCompletionResults pkt_readLevelCompletionResults(struct PacketContext ctx, const uint8_t **pkt) {
	struct LevelCompletionResults out;
	out.gameplayModifiers = pkt_readGameplayModifiers(ctx, pkt);
	out.modifiedScore = pkt_readVarInt32(ctx, pkt);
	out.rawScore = pkt_readVarInt32(ctx, pkt);
	out.rank = pkt_readVarInt32(ctx, pkt);
	out.fullCombo = pkt_readUint8(ctx, pkt);
	out.leftSaberMovementDistance = pkt_readFloat32(ctx, pkt);
	out.rightSaberMovementDistance = pkt_readFloat32(ctx, pkt);
	out.leftHandMovementDistance = pkt_readFloat32(ctx, pkt);
	out.rightHandMovementDistance = pkt_readFloat32(ctx, pkt);
	out.songDuration = pkt_readFloat32(ctx, pkt);
	out.levelEndStateType = pkt_readVarInt32(ctx, pkt);
	out.levelEndAction = pkt_readVarInt32(ctx, pkt);
	out.energy = pkt_readFloat32(ctx, pkt);
	out.goodCutsCount = pkt_readVarInt32(ctx, pkt);
	out.badCutsCount = pkt_readVarInt32(ctx, pkt);
	out.missedCount = pkt_readVarInt32(ctx, pkt);
	out.notGoodCount = pkt_readVarInt32(ctx, pkt);
	out.okCount = pkt_readVarInt32(ctx, pkt);
	out.averageCutScore = pkt_readVarInt32(ctx, pkt);
	out.maxCutScore = pkt_readVarInt32(ctx, pkt);
	out.averageCutDistanceRawScore = pkt_readFloat32(ctx, pkt);
	out.maxCombo = pkt_readVarInt32(ctx, pkt);
	out.minDirDeviation = pkt_readFloat32(ctx, pkt);
	out.maxDirDeviation = pkt_readFloat32(ctx, pkt);
	out.averageDirDeviation = pkt_readFloat32(ctx, pkt);
	out.minTimeDeviation = pkt_readFloat32(ctx, pkt);
	out.maxTimeDeviation = pkt_readFloat32(ctx, pkt);
	out.averageTimeDeviation = pkt_readFloat32(ctx, pkt);
	out.endSongTime = pkt_readFloat32(ctx, pkt);
	return out;
}
struct MultiplayerLevelCompletionResults pkt_readMultiplayerLevelCompletionResults(struct PacketContext ctx, const uint8_t **pkt) {
	struct MultiplayerLevelCompletionResults out;
	if(ctx.protocolVersion <= 6) {
		out.levelEndState = pkt_readVarInt32(ctx, pkt);
	}
	if(ctx.protocolVersion > 6) {
		out.playerLevelEndState = pkt_readVarInt32(ctx, pkt);
		out.playerLevelEndReason = pkt_readVarInt32(ctx, pkt);
	}
	if((ctx.protocolVersion <= 6 && out.levelEndState < MultiplayerLevelEndState_GivenUp) || (ctx.protocolVersion > 6 && out.playerLevelEndState != MultiplayerPlayerLevelEndState_NotStarted)) {
		out.levelCompletionResults = pkt_readLevelCompletionResults(ctx, pkt);
	}
	return out;
}
struct NodePoseSyncState1 pkt_readNodePoseSyncState1(struct PacketContext ctx, const uint8_t **pkt) {
	struct NodePoseSyncState1 out;
	out.head = pkt_readPoseSerializable(ctx, pkt);
	out.leftController = pkt_readPoseSerializable(ctx, pkt);
	out.rightController = pkt_readPoseSerializable(ctx, pkt);
	return out;
}
struct StandardScoreSyncState pkt_readStandardScoreSyncState(struct PacketContext ctx, const uint8_t **pkt) {
	struct StandardScoreSyncState out;
	out.modifiedScore = pkt_readVarInt32(ctx, pkt);
	out.rawScore = pkt_readVarInt32(ctx, pkt);
	out.immediateMaxPossibleRawScore = pkt_readVarInt32(ctx, pkt);
	out.combo = pkt_readVarInt32(ctx, pkt);
	out.multiplier = pkt_readVarInt32(ctx, pkt);
	return out;
}
struct NoteSpawnInfoNetSerializable pkt_readNoteSpawnInfoNetSerializable(struct PacketContext ctx, const uint8_t **pkt) {
	struct NoteSpawnInfoNetSerializable out;
	out.time = pkt_readFloat32(ctx, pkt);
	out.lineIndex = pkt_readVarInt32(ctx, pkt);
	out.noteLineLayer = pkt_readVarInt32(ctx, pkt);
	out.beforeJumpNoteLineLayer = pkt_readVarInt32(ctx, pkt);
	out.colorType = pkt_readVarInt32(ctx, pkt);
	out.cutDirection = pkt_readVarInt32(ctx, pkt);
	out.timeToNextColorNote = pkt_readFloat32(ctx, pkt);
	out.timeToPrevColorNote = pkt_readFloat32(ctx, pkt);
	out.flipLineIndex = pkt_readVarInt32(ctx, pkt);
	out.flipYSide = pkt_readVarInt32(ctx, pkt);
	out.moveStartPos = pkt_readVector3Serializable(ctx, pkt);
	out.moveEndPos = pkt_readVector3Serializable(ctx, pkt);
	out.jumpEndPos = pkt_readVector3Serializable(ctx, pkt);
	out.jumpGravity = pkt_readFloat32(ctx, pkt);
	out.moveDuration = pkt_readFloat32(ctx, pkt);
	out.jumpDuration = pkt_readFloat32(ctx, pkt);
	out.rotation = pkt_readFloat32(ctx, pkt);
	out.cutDirectionAngleOffset = pkt_readFloat32(ctx, pkt);
	return out;
}
struct ObstacleSpawnInfoNetSerializable pkt_readObstacleSpawnInfoNetSerializable(struct PacketContext ctx, const uint8_t **pkt) {
	struct ObstacleSpawnInfoNetSerializable out;
	out.time = pkt_readFloat32(ctx, pkt);
	out.lineIndex = pkt_readVarInt32(ctx, pkt);
	out.obstacleType = pkt_readVarInt32(ctx, pkt);
	out.duration = pkt_readFloat32(ctx, pkt);
	out.width = pkt_readVarInt32(ctx, pkt);
	out.moveStartPos = pkt_readVector3Serializable(ctx, pkt);
	out.moveEndPos = pkt_readVector3Serializable(ctx, pkt);
	out.jumpEndPos = pkt_readVector3Serializable(ctx, pkt);
	out.obstacleHeight = pkt_readFloat32(ctx, pkt);
	out.moveDuration = pkt_readFloat32(ctx, pkt);
	out.jumpDuration = pkt_readFloat32(ctx, pkt);
	out.noteLinesDistance = pkt_readFloat32(ctx, pkt);
	out.rotation = pkt_readFloat32(ctx, pkt);
	return out;
}
void pkt_writeSetPlayersMissingEntitlementsToLevel(struct PacketContext ctx, uint8_t **pkt, struct SetPlayersMissingEntitlementsToLevel in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writePlayersMissingEntitlementsNetSerializable(ctx, pkt, in.playersMissingEntitlements);
	}
}
void pkt_writeGetIsEntitledToLevel(struct PacketContext ctx, uint8_t **pkt, struct GetIsEntitledToLevel in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writeLongString(ctx, pkt, in.levelId);
	}
}
struct SetIsEntitledToLevel pkt_readSetIsEntitledToLevel(struct PacketContext ctx, const uint8_t **pkt) {
	struct SetIsEntitledToLevel out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(ctx, pkt);
	if(out.flags.hasValue0) {
		out.levelId = pkt_readLongString(ctx, pkt);
	}
	if(out.flags.hasValue1) {
		out.entitlementStatus = pkt_readVarInt32(ctx, pkt);
	}
	return out;
}
void pkt_writeSetSelectedBeatmap(struct PacketContext ctx, uint8_t **pkt, struct SetSelectedBeatmap in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writeBeatmapIdentifierNetSerializable(ctx, pkt, in.identifier);
	}
}
struct RecommendBeatmap pkt_readRecommendBeatmap(struct PacketContext ctx, const uint8_t **pkt) {
	struct RecommendBeatmap out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(ctx, pkt);
	if(out.flags.hasValue0) {
		out.identifier = pkt_readBeatmapIdentifierNetSerializable(ctx, pkt);
	}
	return out;
}
void pkt_writeRecommendBeatmap(struct PacketContext ctx, uint8_t **pkt, struct RecommendBeatmap in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writeBeatmapIdentifierNetSerializable(ctx, pkt, in.identifier);
	}
}
struct ClearRecommendedBeatmap pkt_readClearRecommendedBeatmap(struct PacketContext ctx, const uint8_t **pkt) {
	struct ClearRecommendedBeatmap out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	return out;
}
struct GetRecommendedBeatmap pkt_readGetRecommendedBeatmap(struct PacketContext ctx, const uint8_t **pkt) {
	struct GetRecommendedBeatmap out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	return out;
}
void pkt_writeGetRecommendedBeatmap(struct PacketContext ctx, uint8_t **pkt, struct GetRecommendedBeatmap in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
}
void pkt_writeSetSelectedGameplayModifiers(struct PacketContext ctx, uint8_t **pkt, struct SetSelectedGameplayModifiers in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writeGameplayModifiers(ctx, pkt, in.gameplayModifiers);
	}
}
struct RecommendGameplayModifiers pkt_readRecommendGameplayModifiers(struct PacketContext ctx, const uint8_t **pkt) {
	struct RecommendGameplayModifiers out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(ctx, pkt);
	if(out.flags.hasValue0) {
		out.gameplayModifiers = pkt_readGameplayModifiers(ctx, pkt);
	}
	return out;
}
void pkt_writeRecommendGameplayModifiers(struct PacketContext ctx, uint8_t **pkt, struct RecommendGameplayModifiers in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writeGameplayModifiers(ctx, pkt, in.gameplayModifiers);
	}
}
struct ClearRecommendedGameplayModifiers pkt_readClearRecommendedGameplayModifiers(struct PacketContext ctx, const uint8_t **pkt) {
	struct ClearRecommendedGameplayModifiers out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	return out;
}
struct GetRecommendedGameplayModifiers pkt_readGetRecommendedGameplayModifiers(struct PacketContext ctx, const uint8_t **pkt) {
	struct GetRecommendedGameplayModifiers out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	return out;
}
void pkt_writeGetRecommendedGameplayModifiers(struct PacketContext ctx, uint8_t **pkt, struct GetRecommendedGameplayModifiers in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
}
void pkt_writeStartLevel(struct PacketContext ctx, uint8_t **pkt, struct StartLevel in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writeBeatmapIdentifierNetSerializable(ctx, pkt, in.beatmapId);
	}
	if(in.flags.hasValue1) {
		pkt_writeGameplayModifiers(ctx, pkt, in.gameplayModifiers);
	}
	if(in.flags.hasValue2) {
		pkt_writeFloat32(ctx, pkt, in.startTime);
	}
}
struct GetStartedLevel pkt_readGetStartedLevel(struct PacketContext ctx, const uint8_t **pkt) {
	struct GetStartedLevel out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	return out;
}
void pkt_writeCancelLevelStart(struct PacketContext ctx, uint8_t **pkt, struct CancelLevelStart in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
}
struct GetMultiplayerGameState pkt_readGetMultiplayerGameState(struct PacketContext ctx, const uint8_t **pkt) {
	struct GetMultiplayerGameState out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	return out;
}
void pkt_writeSetMultiplayerGameState(struct PacketContext ctx, uint8_t **pkt, struct SetMultiplayerGameState in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writeVarInt32(ctx, pkt, in.lobbyState);
	}
}
struct GetIsReady pkt_readGetIsReady(struct PacketContext ctx, const uint8_t **pkt) {
	struct GetIsReady out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	return out;
}
void pkt_writeGetIsReady(struct PacketContext ctx, uint8_t **pkt, struct GetIsReady in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
}
struct SetIsReady pkt_readSetIsReady(struct PacketContext ctx, const uint8_t **pkt) {
	struct SetIsReady out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(ctx, pkt);
	if(out.flags.hasValue0) {
		out.isReady = pkt_readUint8(ctx, pkt);
	}
	return out;
}
void pkt_writeSetIsReady(struct PacketContext ctx, uint8_t **pkt, struct SetIsReady in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writeUint8(ctx, pkt, in.isReady);
	}
}
struct GetIsInLobby pkt_readGetIsInLobby(struct PacketContext ctx, const uint8_t **pkt) {
	struct GetIsInLobby out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	return out;
}
void pkt_writeGetIsInLobby(struct PacketContext ctx, uint8_t **pkt, struct GetIsInLobby in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
}
struct SetIsInLobby pkt_readSetIsInLobby(struct PacketContext ctx, const uint8_t **pkt) {
	struct SetIsInLobby out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(ctx, pkt);
	if(out.flags.hasValue0) {
		out.isBack = pkt_readUint8(ctx, pkt);
	}
	return out;
}
void pkt_writeSetIsInLobby(struct PacketContext ctx, uint8_t **pkt, struct SetIsInLobby in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writeUint8(ctx, pkt, in.isBack);
	}
}
struct GetCountdownEndTime pkt_readGetCountdownEndTime(struct PacketContext ctx, const uint8_t **pkt) {
	struct GetCountdownEndTime out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	return out;
}
void pkt_writeSetCountdownEndTime(struct PacketContext ctx, uint8_t **pkt, struct SetCountdownEndTime in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writeFloat32(ctx, pkt, in.newTime);
	}
}
void pkt_writeCancelCountdown(struct PacketContext ctx, uint8_t **pkt, struct CancelCountdown in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
}
void pkt_writeGetOwnedSongPacks(struct PacketContext ctx, uint8_t **pkt, struct GetOwnedSongPacks in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
}
struct SetOwnedSongPacks pkt_readSetOwnedSongPacks(struct PacketContext ctx, const uint8_t **pkt) {
	struct SetOwnedSongPacks out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(ctx, pkt);
	if(out.flags.hasValue0) {
		out.songPackMask = pkt_readSongPackMask(ctx, pkt);
	}
	return out;
}
struct GetPermissionConfiguration pkt_readGetPermissionConfiguration(struct PacketContext ctx, const uint8_t **pkt) {
	struct GetPermissionConfiguration out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	return out;
}
void pkt_writeSetPermissionConfiguration(struct PacketContext ctx, uint8_t **pkt, struct SetPermissionConfiguration in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writePlayersLobbyPermissionConfigurationNetSerializable(ctx, pkt, in.playersPermissionConfiguration);
	}
}
void pkt_writeSetIsStartButtonEnabled(struct PacketContext ctx, uint8_t **pkt, struct SetIsStartButtonEnabled in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writeVarInt32(ctx, pkt, in.reason);
	}
}
void pkt_writeSetGameplaySceneSyncFinish(struct PacketContext ctx, uint8_t **pkt, struct SetGameplaySceneSyncFinish in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writePlayerSpecificSettingsAtStartNetSerializable(ctx, pkt, in.playersAtGameStart);
	}
	if(in.flags.hasValue1) {
		pkt_writeString(ctx, pkt, in.sessionGameId);
	}
}
struct SetGameplaySceneReady pkt_readSetGameplaySceneReady(struct PacketContext ctx, const uint8_t **pkt) {
	struct SetGameplaySceneReady out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(ctx, pkt);
	if(out.flags.hasValue0) {
		out.playerSpecificSettingsNetSerializable = pkt_readPlayerSpecificSettingsNetSerializable(ctx, pkt);
	}
	return out;
}
void pkt_writeGetGameplaySceneReady(struct PacketContext ctx, uint8_t **pkt, struct GetGameplaySceneReady in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
}
struct SetGameplaySongReady pkt_readSetGameplaySongReady(struct PacketContext ctx, const uint8_t **pkt) {
	struct SetGameplaySongReady out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	return out;
}
void pkt_writeGetGameplaySongReady(struct PacketContext ctx, uint8_t **pkt, struct GetGameplaySongReady in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
}
void pkt_writeSetSongStartTime(struct PacketContext ctx, uint8_t **pkt, struct SetSongStartTime in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
	pkt_writeRemoteProcedureCallFlags(ctx, pkt, in.flags);
	if(in.flags.hasValue0) {
		pkt_writeFloat32(ctx, pkt, in.startTime);
	}
}
struct NoteCut pkt_readNoteCut(struct PacketContext ctx, const uint8_t **pkt) {
	struct NoteCut out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(ctx, pkt);
	if(out.flags.hasValue0) {
		out.songTime = pkt_readFloat32(ctx, pkt);
	}
	if(out.flags.hasValue1) {
		out.noteCutInfo = pkt_readNoteCutInfoNetSerializable(ctx, pkt);
	}
	return out;
}
struct NoteMissed pkt_readNoteMissed(struct PacketContext ctx, const uint8_t **pkt) {
	struct NoteMissed out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(ctx, pkt);
	if(out.flags.hasValue0) {
		out.songTime = pkt_readFloat32(ctx, pkt);
	}
	if(out.flags.hasValue1) {
		out.noteMissInfo = pkt_readNoteMissInfoNetSerializable(ctx, pkt);
	}
	return out;
}
struct LevelFinished pkt_readLevelFinished(struct PacketContext ctx, const uint8_t **pkt) {
	struct LevelFinished out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(ctx, pkt);
	if(out.flags.hasValue0) {
		out.results = pkt_readMultiplayerLevelCompletionResults(ctx, pkt);
	}
	return out;
}
void pkt_writeReturnToMenu(struct PacketContext ctx, uint8_t **pkt, struct ReturnToMenu in) {
	pkt_writeRemoteProcedureCall(ctx, pkt, in.base);
}
struct RequestReturnToMenu pkt_readRequestReturnToMenu(struct PacketContext ctx, const uint8_t **pkt) {
	struct RequestReturnToMenu out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	return out;
}
struct NoteSpawned pkt_readNoteSpawned(struct PacketContext ctx, const uint8_t **pkt) {
	struct NoteSpawned out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(ctx, pkt);
	if(out.flags.hasValue0) {
		out.songTime = pkt_readFloat32(ctx, pkt);
	}
	if(out.flags.hasValue1) {
		out.noteSpawnInfo = pkt_readNoteSpawnInfoNetSerializable(ctx, pkt);
	}
	return out;
}
struct ObstacleSpawned pkt_readObstacleSpawned(struct PacketContext ctx, const uint8_t **pkt) {
	struct ObstacleSpawned out;
	out.base = pkt_readRemoteProcedureCall(ctx, pkt);
	out.flags = pkt_readRemoteProcedureCallFlags(ctx, pkt);
	if(out.flags.hasValue0) {
		out.songTime = pkt_readFloat32(ctx, pkt);
	}
	if(out.flags.hasValue1) {
		out.obstacleSpawnInfo = pkt_readObstacleSpawnInfoNetSerializable(ctx, pkt);
	}
	return out;
}
struct NodePoseSyncState pkt_readNodePoseSyncState(struct PacketContext ctx, const uint8_t **pkt) {
	struct NodePoseSyncState out;
	out.id = pkt_readSyncStateId(ctx, pkt);
	out.time = pkt_readFloat32(ctx, pkt);
	out.state = pkt_readNodePoseSyncState1(ctx, pkt);
	return out;
}
struct ScoreSyncState pkt_readScoreSyncState(struct PacketContext ctx, const uint8_t **pkt) {
	struct ScoreSyncState out;
	out.id = pkt_readSyncStateId(ctx, pkt);
	out.time = pkt_readFloat32(ctx, pkt);
	out.state = pkt_readStandardScoreSyncState(ctx, pkt);
	return out;
}
struct NodePoseSyncStateDelta pkt_readNodePoseSyncStateDelta(struct PacketContext ctx, const uint8_t **pkt) {
	struct NodePoseSyncStateDelta out;
	out.baseId = pkt_readSyncStateId(ctx, pkt);
	out.timeOffsetMs = pkt_readVarInt32(ctx, pkt);
	if(out.baseId.same == 0) {
		out.delta = pkt_readNodePoseSyncState1(ctx, pkt);
	}
	return out;
}
struct ScoreSyncStateDelta pkt_readScoreSyncStateDelta(struct PacketContext ctx, const uint8_t **pkt) {
	struct ScoreSyncStateDelta out;
	out.baseId = pkt_readSyncStateId(ctx, pkt);
	out.timeOffsetMs = pkt_readVarInt32(ctx, pkt);
	if(out.baseId.same == 0) {
		out.delta = pkt_readStandardScoreSyncState(ctx, pkt);
	}
	return out;
}
struct MpCore pkt_readMpCore(struct PacketContext ctx, const uint8_t **pkt) {
	struct MpCore out;
	out.packetType = pkt_readString(ctx, pkt);
	return out;
}
struct MultiplayerSessionMessageHeader pkt_readMultiplayerSessionMessageHeader(struct PacketContext ctx, const uint8_t **pkt) {
	struct MultiplayerSessionMessageHeader out;
	out.type = pkt_readUint8(ctx, pkt);
	return out;
}
void pkt_writeMultiplayerSessionMessageHeader(struct PacketContext ctx, uint8_t **pkt, struct MultiplayerSessionMessageHeader in) {
	pkt_writeUint8(ctx, pkt, in.type);
}
struct MenuRpcHeader pkt_readMenuRpcHeader(struct PacketContext ctx, const uint8_t **pkt) {
	struct MenuRpcHeader out;
	out.type = pkt_readUint8(ctx, pkt);
	return out;
}
void pkt_writeMenuRpcHeader(struct PacketContext ctx, uint8_t **pkt, struct MenuRpcHeader in) {
	pkt_writeUint8(ctx, pkt, in.type);
}
struct GameplayRpcHeader pkt_readGameplayRpcHeader(struct PacketContext ctx, const uint8_t **pkt) {
	struct GameplayRpcHeader out;
	out.type = pkt_readUint8(ctx, pkt);
	return out;
}
void pkt_writeGameplayRpcHeader(struct PacketContext ctx, uint8_t **pkt, struct GameplayRpcHeader in) {
	pkt_writeUint8(ctx, pkt, in.type);
}
struct MpBeatmapPacket pkt_readMpBeatmapPacket(struct PacketContext ctx, const uint8_t **pkt) {
	struct MpBeatmapPacket out;
	out.levelHash = pkt_readString(ctx, pkt);
	out.songName = pkt_readLongString(ctx, pkt);
	out.songSubName = pkt_readLongString(ctx, pkt);
	out.songAuthorName = pkt_readLongString(ctx, pkt);
	out.levelAuthorName = pkt_readLongString(ctx, pkt);
	out.beatsPerMinute = pkt_readFloat32(ctx, pkt);
	out.songDuration = pkt_readFloat32(ctx, pkt);
	out.characteristic = pkt_readString(ctx, pkt);
	out.difficulty = pkt_readUint32(ctx, pkt);
	return out;
}
struct MpPlayerData pkt_readMpPlayerData(struct PacketContext ctx, const uint8_t **pkt) {
	struct MpPlayerData out;
	out.platformId = pkt_readString(ctx, pkt);
	out.platform = pkt_readInt32(ctx, pkt);
	return out;
}
struct AuthenticateUserRequest pkt_readAuthenticateUserRequest(struct PacketContext ctx, const uint8_t **pkt) {
	struct AuthenticateUserRequest out;
	out.base = pkt_readBaseMasterServerReliableResponse(ctx, pkt);
	out.authenticationToken = pkt_readAuthenticationToken(ctx, pkt);
	return out;
}
void pkt_writeAuthenticateUserResponse(struct PacketContext ctx, uint8_t **pkt, struct AuthenticateUserResponse in) {
	pkt_writeBaseMasterServerReliableResponse(ctx, pkt, in.base);
	pkt_writeUint8(ctx, pkt, in.result);
}
void pkt_writeConnectToServerResponse(struct PacketContext ctx, uint8_t **pkt, struct ConnectToServerResponse in) {
	pkt_writeBaseMasterServerReliableResponse(ctx, pkt, in.base);
	pkt_writeUint8(ctx, pkt, in.result);
	if(in.result == GetPublicServersResponse_Result_Success) {
		pkt_writeString(ctx, pkt, in.userId);
		pkt_writeString(ctx, pkt, in.userName);
		pkt_writeString(ctx, pkt, in.secret);
		pkt_writeBeatmapLevelSelectionMask(ctx, pkt, in.selectionMask);
		pkt_writeUint8(ctx, pkt, in.flags);
		pkt_writeIPEndPoint(ctx, pkt, in.remoteEndPoint);
		pkt_writeUint8Array(ctx, pkt, in.random, 32);
		pkt_writeByteArrayNetSerializable(ctx, pkt, in.publicKey);
		pkt_writeServerCode(ctx, pkt, in.code);
		pkt_writeGameplayServerConfiguration(ctx, pkt, in.configuration);
		pkt_writeString(ctx, pkt, in.managerId);
	}
}
struct ConnectToServerRequest pkt_readConnectToServerRequest(struct PacketContext ctx, const uint8_t **pkt) {
	struct ConnectToServerRequest out;
	out.base = pkt_readBaseConnectToServerRequest(ctx, pkt);
	out.selectionMask = pkt_readBeatmapLevelSelectionMask(ctx, pkt);
	out.secret = pkt_readString(ctx, pkt);
	out.code = pkt_readServerCode(ctx, pkt);
	out.configuration = pkt_readGameplayServerConfiguration(ctx, pkt);
	return out;
}
struct ClientHelloRequest pkt_readClientHelloRequest(struct PacketContext ctx, const uint8_t **pkt) {
	struct ClientHelloRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(ctx, pkt);
	pkt_readUint8Array(pkt, out.random, 32);
	return out;
}
void pkt_writeHelloVerifyRequest(struct PacketContext ctx, uint8_t **pkt, struct HelloVerifyRequest in) {
	pkt_writeBaseMasterServerReliableResponse(ctx, pkt, in.base);
	pkt_writeUint8Array(ctx, pkt, in.cookie, 32);
}
struct ClientHelloWithCookieRequest pkt_readClientHelloWithCookieRequest(struct PacketContext ctx, const uint8_t **pkt) {
	struct ClientHelloWithCookieRequest out;
	out.base = pkt_readBaseMasterServerReliableRequest(ctx, pkt);
	out.certificateResponseId = pkt_readUint32(ctx, pkt);
	pkt_readUint8Array(pkt, out.random, 32);
	pkt_readUint8Array(pkt, out.cookie, 32);
	return out;
}
void pkt_writeServerHelloRequest(struct PacketContext ctx, uint8_t **pkt, struct ServerHelloRequest in) {
	pkt_writeBaseMasterServerReliableResponse(ctx, pkt, in.base);
	pkt_writeUint8Array(ctx, pkt, in.random, 32);
	pkt_writeByteArrayNetSerializable(ctx, pkt, in.publicKey);
	pkt_writeByteArrayNetSerializable(ctx, pkt, in.signature);
}
void pkt_writeServerCertificateRequest(struct PacketContext ctx, uint8_t **pkt, struct ServerCertificateRequest in) {
	pkt_writeBaseMasterServerReliableResponse(ctx, pkt, in.base);
	pkt_writeVarUint32(ctx, pkt, in.certificateCount);
	for(uint32_t i = 0; i < in.certificateCount; ++i)
		pkt_writeByteArrayNetSerializable(ctx, pkt, in.certificateList[i]);
}
struct ClientKeyExchangeRequest pkt_readClientKeyExchangeRequest(struct PacketContext ctx, const uint8_t **pkt) {
	struct ClientKeyExchangeRequest out;
	out.base = pkt_readBaseMasterServerReliableResponse(ctx, pkt);
	out.clientPublicKey = pkt_readByteArrayNetSerializable(ctx, pkt);
	return out;
}
void pkt_writeChangeCipherSpecRequest(struct PacketContext ctx, uint8_t **pkt, struct ChangeCipherSpecRequest in) {
	pkt_writeBaseMasterServerReliableResponse(ctx, pkt, in.base);
}
void pkt_logMessageType(struct PacketContext ctx, const char *name, char *buf, char *it, MessageType in) {
	fprintf(stderr, "%.*s%s=%u (%s)\n", (uint32_t)(it - buf), buf, name, in, reflect(MessageType, in));
}
struct MessageHeader pkt_readMessageHeader(struct PacketContext ctx, const uint8_t **pkt) {
	struct MessageHeader out;
	out.type = pkt_readUint32(ctx, pkt);
	out.protocolVersion = pkt_readVarUint32(ctx, pkt);
	return out;
}
void pkt_writeMessageHeader(struct PacketContext ctx, uint8_t **pkt, struct MessageHeader in) {
	pkt_writeUint32(ctx, pkt, in.type);
	pkt_writeVarUint32(ctx, pkt, in.protocolVersion);
}
struct SerializeHeader pkt_readSerializeHeader(struct PacketContext ctx, const uint8_t **pkt) {
	struct SerializeHeader out;
	out.length = pkt_readVarUint32(ctx, pkt);
	out.type = pkt_readUint8(ctx, pkt);
	return out;
}
void pkt_writeSerializeHeader(struct PacketContext ctx, uint8_t **pkt, struct SerializeHeader in) {
	pkt_writeVarUint32(ctx, pkt, in.length);
	pkt_writeUint8(ctx, pkt, in.type);
}
