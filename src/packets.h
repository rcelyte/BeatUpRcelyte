#pragma once
#include "../common/packets.h"

struct RemoteProcedureCallFlags {
	bool hasValue0, hasValue1, hasValue2, hasValue3;
};

uint8_t MpCoreType_From(const struct String *type);

#include "packets.gen.h"

static const ServerCode ServerCode_NONE = 0;
ServerCode StringToServerCode(const char *in, uint32_t len);
char *ServerCodeToString(char *out, ServerCode in);
