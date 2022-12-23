#pragma once
#include "../common/packets.h"

struct RemoteProcedureCallFlags {
	bool hasValue0, hasValue1, hasValue2, hasValue3;
};

uint8_t MpCoreType_From(const struct String *type);

#include "packets.gen.h"

bool _pkt_serialize(PacketWriteFunc inner, const void *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
#define pkt_serialize(data, pkt, end, version) _pkt_serialize(_pkt_write_func(data), data, pkt, end, version)
bool pkt_debug(const char *errorMessage, const uint8_t *head, const uint8_t *end, size_t expect, struct PacketContext ctx);

static const ServerCode ServerCode_NONE = 0;
ServerCode StringToServerCode(const char *in, uint32_t len);
char *ServerCodeToString(char *out, ServerCode in);
