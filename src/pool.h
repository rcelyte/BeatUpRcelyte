#include "packets.h"

struct RoomHandle {
	uint16_t block;
	uint8_t sub, high;
};

_Bool pool_init();
_Bool pool_request_room(struct RoomHandle *out);
void pool_room_close(struct RoomHandle h);
ServerCode pool_room_code(struct RoomHandle h);
_Bool pool_find_room(ServerCode code, struct RoomHandle *out);
