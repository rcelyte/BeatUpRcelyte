#include "wire.h"

static _Bool open = 0;
_Bool wire_request_block(struct WireBlockHandle *block) {
	if(open)
		return 1;
	open = 1;
	*block = (struct WireBlockHandle){0,0,0};
	return 0;
}

void wire_block_release(struct WireBlockHandle block) {
	open = 0;
}
