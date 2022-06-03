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

#define String_eq(a, b) ((a).length == (b).length && memcmp((a).data, (b).data, (b).length) == 0)
#define String_is(a, str) ((a).length == (lengthof(str) - 1) && memcmp((a).data, str, (lengthof(str) - 1)) == 0)
#ifdef __cplusplus
#define String_from(str) {lengthof(str) - 1, false, str}
#else
#define String_from(str) (struct String){lengthof(str) - 1, false, str}
#endif
