#include <stdint.h>

struct WireBlockHandle {
	uint16_t host, thread, group;
};

_Bool wire_request_block(struct WireBlockHandle *block);
void wire_block_release(struct WireBlockHandle);
