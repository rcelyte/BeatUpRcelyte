#include "packets.h"
#include <stdio.h>

static _Bool IsInvalidLength(size_t length) {
	if(length < 36)
		return 1;
	if((length - 4 - 16) % 16)
		return 1;
	return 0;
}

struct PacketEncryptionLayer pkt_readPacketEncryptionLayer(uint8_t **pkt, uint8_t *end, uint8_t serverRandom[32], uint8_t clientRandom[32]) {
	struct PacketEncryptionLayer out;
	out.encrypted = pkt_readUint8(pkt);
	if(out.encrypted != 1)
		return out;
	if(IsInvalidLength(end - *pkt))
		return out;
	// TODO: the rest of the function
	// out.encrypted = 0;
	return out;
}
void pkt_writePacketEncryptionLayer(uint8_t **pkt, struct PacketEncryptionLayer in) {
	pkt_writeUint8(pkt, in.encrypted);
	if(in.encrypted)
		fprintf(stderr, "WRITE ENCRYPTED PACKET\n");
}