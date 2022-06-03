#include "../common/packets.h.h"

struct ExString {
	struct String base;
	uint8_t tier;
};
struct RemoteProcedureCallFlags {
	bool hasValue0, hasValue1, hasValue2, hasValue3;
};

uint8_t MpCoreType_From(const struct String *type);
