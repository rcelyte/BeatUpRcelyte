#include "global.h"
#include "packets.h"
#include "../common/packets.c"

bool _pkt_serialize(PacketWriteFunc inner, const void *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint8_t *start = (*pkt)++;
	struct SerializeHeader header = {
		.length = (uint32_t)_pkt_try_write(inner, data, pkt, end, ctx),
	};
	if(header.length) {
		uint8_t *retry = start;
		if(pkt_write(&header, &retry, end, ctx) == 1)
			return true;
		*pkt = retry;
		if(_pkt_try_write(inner, data, pkt, end, ctx))
			return true;
	}
	(*pkt) = start;
	return false;
}

bool pkt_debug(const char *errorMessage, const uint8_t *head, const uint8_t *end, size_t expect, struct PacketContext ctx) {
	if(head == end)
		return false;
	const uint8_t *start = end - expect;
	uprintf("%s [netVersion=%hhu, protocolVersion=%hhu, beatUpVersion=%hhu, windowSize=%u] (expected %zu, read %zu)\n    ", errorMessage, ctx.netVersion, ctx.protocolVersion, ctx.beatUpVersion, ctx.windowSize, expect, head - start);
	for(const uint8_t *it = start; it < end; ++it)
		uprintf("%02hhx", *it);
	if((uintptr_t)(head - start) < expect) {
		uprintf("\n    ");
		for(const uint8_t *it = start; it < head; ++it)
			uprintf("  ");
		uprintf("^ extra data starts here");
	}
	uprintf("\n");
	return true;
}

ServerCode StringToServerCode(const char *in, uint32_t len) {
	static const uint8_t readTable[128] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,27,28,29,30,31,32,33,34,35,36,1,1,1,1,1,1,1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	ServerCode out = 0;
	if(len <= 5)
		for(uint32_t i = 0, fac = 1; i < len; ++i, fac *= 36)
			out += readTable[in[i] & 127] * fac;
	return out;
}

char *ServerCodeToString(char *out, ServerCode in) {
	char *s = out;
	for(; in; in /= 36)
		*s++ = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[--in % 36];
	*s = 0;
	return out;
}

[[maybe_unused]] static void _pkt_ServerCode_read(ServerCode *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	struct String str;
	_pkt_String_read(&str, pkt, end, ctx);
	*data = StringToServerCode(str.data, str.length);
}
[[maybe_unused]] static void _pkt_ServerCode_write(const ServerCode *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	struct String str = {.length = 0, .isNull = false};
	for(ServerCode code = *data; code; code /= 36)
		str.data[str.length++] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[--code % 36];
	_pkt_String_write(&str, pkt, end, ctx);
}
[[maybe_unused]] static void _pkt_RemoteProcedureCallFlags_read(struct RemoteProcedureCallFlags *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint8_t bits = 0xff;
	if(ctx.protocolVersion > 6)
		_pkt_u8_read(&bits, pkt, end, ctx);
	data->hasValue0 = (bits >> 0) & 1;
	data->hasValue1 = (bits >> 1) & 1;
	data->hasValue2 = (bits >> 2) & 1;
	data->hasValue3 = (bits >> 3) & 1;
}
[[maybe_unused]] static void _pkt_RemoteProcedureCallFlags_write(const struct RemoteProcedureCallFlags *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	if(ctx.protocolVersion > 6) {
		uint8_t bits = 0;
		bits |= (data->hasValue0 << 0);
		bits |= (data->hasValue1 << 1);
		bits |= (data->hasValue2 << 2);
		bits |= (data->hasValue3 << 3);
		_pkt_u8_write(&bits, pkt, end, ctx);
	}
}
MpCoreType MpCoreType_From(const struct String *type) {
	if(String_is(*type, "MpBeatmapPacket"))
		return MpCoreType_MpBeatmapPacket;
	if(String_is(*type, "MpPlayerData"))
		return MpCoreType_MpPlayerData;
	if(String_is(*type, "CustomAvatarPacket"))
		return MpCoreType_CustomAvatarPacket;
	uprintf("Unsupported MpCore custom packet: %.*s\n", type->length, type->data);
	return (MpCoreType)0xffffffff;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "packets.gen.c.h"
