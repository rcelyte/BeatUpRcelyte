#define pkt_serialize(data, pkt, end, version) ( \
	pkt_write_c(pkt, end, version, SerializeHeader, { \
		.length = pkt_write(data, &(uint8_t*){*(pkt)}, end, version), \
	}) && pkt_write(data, pkt, end, version) \
)
struct String {
	uint32_t length;
	bool isNull;
	char data[60];
};
struct LongString {
	uint32_t length;
	bool isNull;
	char data[4096];
};
struct ExString {
	struct String base;
	uint8_t tier;
};
struct RemoteProcedureCallFlags {
	bool hasValue0, hasValue1, hasValue2, hasValue3;
};

#define String_eq(a, b) ((a).length == (b).length && memcmp((a).data, (b).data, (b).length) == 0)
#define String_is(a, str) ((a).length == (lengthof(str) - 1) && memcmp((a).data, str, (lengthof(str) - 1)) == 0)
uint8_t MpCoreType_From(const struct String *type);
