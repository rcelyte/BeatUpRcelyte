#include <stdlib.h>
#include <string.h>

static uint8_t pkt_readUint8(uint8_t **pkt) {
	uint8_t v = (*pkt)[0];
	*pkt += sizeof(v);
	return v;
}
static uint16_t pkt_readUint16(uint8_t **pkt) {
	uint16_t v = (*pkt)[0] | (*pkt)[1] << 8;
	*pkt += sizeof(v);
	return v;
}
static uint32_t pkt_readUint32(uint8_t **pkt) {
	uint32_t v = (*pkt)[0] | (*pkt)[1] << 8 | (*pkt)[2] << 16 | (*pkt)[3] << 24;
	*pkt += sizeof(v);
	return v;
}
static uint64_t pkt_readUint64(uint8_t **pkt) {
	uint64_t v = (uint64_t)(*pkt)[0] | (uint64_t)(*pkt)[1] << 8 | (uint64_t)(*pkt)[2] << 16 | (uint64_t)(*pkt)[3] << 24 | (uint64_t)(*pkt)[4] << 32 | (uint64_t)(*pkt)[5] << 40 | (uint64_t)(*pkt)[6] << 48 | (uint64_t)(*pkt)[7] << 56;
	*pkt += sizeof(v);
	return v;
}
/*static uint64_t pkt_readVarUint64(uint8_t **pkt) {
	uint64_t byte, value = 0;
	uint32_t shift = 0;
	for(; (byte = (uint64_t)pkt_readUint8(pkt)) & 128; shift += 7)
		value |= (byte & 127) << shift;
	return value | byte << shift;
}*/
static uint64_t pkt_readVarUint64(uint8_t **pkt) {
	uint64_t value = 0, byte;
	uint8_t shift = 0;
	do {
		byte = pkt_readUint8(pkt);
		value |= (byte & 127) << shift;
		shift += 7;
	} while(byte & 128);
	return value;
}
static int64_t pkt_readVarInt64(uint8_t **pkt) {
	int64_t varULong = (int64_t)pkt_readVarUint64(pkt);
	if((varULong & 1L) != 1L)
		return varULong >> 1;
	return -(varULong >> 1) + 1L;
}
static uint32_t pkt_readVarUint32(uint8_t **pkt) {
	return (uint32_t)pkt_readVarUint64(pkt);
}
static int32_t pkt_readVarInt32(uint8_t **pkt) {
	return (int32_t)pkt_readVarInt64(pkt);
}
static void pkt_readBytes(uint8_t **pkt, uint8_t *out, uint32_t count) {
	memcpy(out, *pkt, count);
	*pkt += count;
}
static void pkt_writeUint8(uint8_t **pkt, uint8_t v) {
	(*pkt)[0] = v;
	*pkt += sizeof(v);
}
static void pkt_writeUint16(uint8_t **pkt, uint16_t v) {
	(*pkt)[0] = v & 255;
	(*pkt)[1] = v >> 8 & 255;
	*pkt += sizeof(v);
}
static void pkt_writeUint32(uint8_t **pkt, uint32_t v) {
	(*pkt)[0] = v & 255;
	(*pkt)[1] = v >> 8 & 255;
	(*pkt)[2] = v >> 16 & 255;
	(*pkt)[3] = v >> 24 & 255;
	*pkt += sizeof(v);
}
static void pkt_writeUint64(uint8_t **pkt, uint64_t v) {
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
static void pkt_writeVarUint64(uint8_t **pkt, uint64_t v) {
	do {
		uint8_t byte = v & 127;
		v >>= 7;
		if(v)
			byte |= 128;
		pkt_writeUint8(pkt, byte);
	} while(v);
}
static void pkt_writeVarUint32(uint8_t **pkt, uint32_t v) {
	pkt_writeVarUint64(pkt, v);
}
static void pkt_writeBytes(uint8_t **pkt, uint8_t *in, uint32_t count) {
	memcpy(*pkt, in, count);
	*pkt += count;
}
