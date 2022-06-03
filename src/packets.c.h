#include "global.h"
#include "../common/packets.c.h"
#include "scramble.h"

[[maybe_unused]] static void _pkt_ExString_read(struct ExString *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_read(&data->base, pkt, end, ctx);
	data->tier = 0;
}
[[maybe_unused]] static void _pkt_ExString_write(const struct ExString *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	struct String str = data->base;
	if(ctx.beatUpVersion)
		str.data[str.length++] = data->tier + 16;
	_pkt_String_write(&str, pkt, end, ctx);
}

[[maybe_unused]] static void _pkt_ServerCode_read(ServerCode *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	struct String str;
	_pkt_String_read(&str, pkt, end, ctx);
	*data = StringToServerCode(str.data, str.length);
}
[[maybe_unused]] static void _pkt_ServerCode_write(const ServerCode *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	struct String str = {.length = 0, .isNull = 0};
	for(ServerCode code = scramble_encode(*data); code; code /= 36)
		str.data[str.length++] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[--code % 36];
	_pkt_String_write(&str, pkt, end, ctx);
}
[[maybe_unused]] static void _pkt_RemoteProcedureCallFlags_read(struct RemoteProcedureCallFlags *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint8_t bits = ~0;
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
	return ~0;
}
