#include "packets.h"
#include "wire.h"

struct RoomHandle {
	uint16_t block;
	uint8_t sub, high;
};

bool pool_init();
bool pool_request_room(struct RoomHandle *room_out, struct WireRoomHandle *handle_out, struct String managerId, struct GameplayServerConfiguration configuration);
void pool_room_close(struct RoomHandle room);
void pool_room_close_notify(struct RoomHandle room);
ServerCode pool_room_code(struct RoomHandle room);
bool pool_find_room(ServerCode code, struct RoomHandle *room_out, struct WireRoomHandle *handle_out);
