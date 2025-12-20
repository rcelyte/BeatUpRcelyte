#include "global.h"
#include "packets.h"
#include "../common/packets.c"

bool _pkt_serialize(PacketWriteFunc *const inner, const void *const restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
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

bool pkt_debug(const char *const errorMessage, const uint8_t *const start, const uint8_t *const end, const uint8_t *head, const struct PacketContext ctx) {
	if(head == end)
		return false;
	pkt_dump(errorMessage, start, (struct PacketRead){.head = &head, .end = end, .context = ctx});
	return true;
}

ServerCode StringToServerCode(const char *in, uint32_t len) {
	static const uint8_t readTable[64] = {
		1,11,9,12,1,13,14,15,16,2,17,18,19,20,21,1,22,23,24,25,26,27,28,29,30,31,32,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,4,5,6,7,8,9,10,1,1,1,1,1,1,
	};
	ServerCode out = 0;
	if(len <= 5)
		for(uint32_t i = 0, fac = 1; i < len; ++i, fac *= 32)
			out += readTable[in[i] & 63] * fac;
	return out;
}

char *ServerCodeToString(char out[8], ServerCode in) {
	char *s = out;
	for(; in; in >>= 5)
		*s++ = "0123456789ACEFGHJKLMNPQRSTUVWXYZ"[--in & 0x1f];
	*s = 0;
	return out;
}

[[maybe_unused]] static void _pkt_ServerCode_read(ServerCode *const restrict data, const struct PacketRead parent) {
	const struct PacketRead state = scope(parent, "ServerCode");
	struct String str;
	_pkt_String_read(&str, state);
	*data = StringToServerCode(str.data, str.length);
}
[[maybe_unused]] static void _pkt_ServerCode_write(const ServerCode *const restrict data, const struct PacketWrite parent) {
	const struct PacketWrite state = scope(parent, "ServerCode");
	struct String str = {.length = 0, .isNull = false};
	str.length = (uint32_t)strlen(ServerCodeToString(str.data, *data));
	_pkt_String_write(&str, state);
}
[[maybe_unused]] static void _pkt_RemoteProcedureCallFlags_read(struct RemoteProcedureCallFlags *const restrict data, const struct PacketRead parent) {
	const struct PacketRead state = scope(parent, "RemoteProcedureCallFlags");
	uint8_t bits = 0xff;
	if(state.context.protocolVersion > 6)
		_pkt_u8_read(&bits, state);
	data->hasValue0 = (bits >> 0) & 1;
	data->hasValue1 = (bits >> 1) & 1;
	data->hasValue2 = (bits >> 2) & 1;
	data->hasValue3 = (bits >> 3) & 1;
}
[[maybe_unused]] static void _pkt_RemoteProcedureCallFlags_write(const struct RemoteProcedureCallFlags *const restrict data, const struct PacketWrite parent) {
	const struct PacketWrite state = scope(parent, "RemoteProcedureCallFlags");
	if(state.context.protocolVersion > 6) {
		uint8_t bits = 0;
		bits |= (data->hasValue0 << 0);
		bits |= (data->hasValue1 << 1);
		bits |= (data->hasValue2 << 2);
		bits |= (data->hasValue3 << 3);
		_pkt_u8_write(&bits, state);
	}
}
MpCoreType MpCoreType_From(const struct String *type) {
	if(String_is(*type, "MpcTextChatPacket")) return MpCoreType_MpcTextChatPacket;
	if(String_is(*type, "MpBeatmapPacket")) return MpCoreType_MpBeatmapPacket;
	if(String_is(*type, "MpPerPlayerPacket")) return MpCoreType_MpPerPlayerPacket;
	if(String_is(*type, "GetMpPerPlayerPacket")) return MpCoreType_GetMpPerPlayerPacket;
	if(String_is(*type, "CustomAvatarPacket")) return MpCoreType_CustomAvatarPacket;
	if(String_is(*type, "MpcCapabilitiesPacket")) return MpCoreType_MpcCapabilitiesPacket;
	if(String_is(*type, "MpPlayerData")) return MpCoreType_MpPlayerData;
	if(String_is(*type, "MpexPlayerData")) return MpCoreType_MpexPlayerData;
	uprintf("Unsupported MpCore custom packet: %.*s\n", type->length, type->data);
	return (MpCoreType)0xffffffff;
}
static void _pkt_InternalMessageType_Fixed_read(InternalMessageType_Fixed *const restrict value, const struct PacketRead parent) {
	const struct PacketRead state = scope(parent, "InternalMessageType");
	uint8_t raw = 0;
	_pkt_u8_read(&raw, state);
	*value = raw + (state.context.gameVersion >= GameVersion_1_42_0 && raw >= InternalMessageType_PlayerAvatarUpdate);
}
static void _pkt_InternalMessageType_Fixed_write(const InternalMessageType_Fixed *const restrict value, const struct PacketWrite parent) {
	const struct PacketWrite state = scope(parent, "InternalMessageType");
	const uint8_t raw = *value - (state.context.gameVersion >= GameVersion_1_42_0 && *value > InternalMessageType_PlayerAvatarUpdate);
	_pkt_u8_write(&raw, state);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "packets.gen.c.h"
