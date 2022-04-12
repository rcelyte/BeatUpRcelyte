#include <stdint.h>
#include "net.h"

struct WireBlockHandle {
	uint16_t host, thread, group;
};

struct WireRoomHandle {
	struct WireBlockHandle block;
	uint8_t sub;
};

_Bool wire_request_block(struct WireBlockHandle *block_out, uint16_t notify);
void wire_block_release(struct WireBlockHandle block);

_Bool wire_room_open(struct WireRoomHandle handle, struct String managerId, struct GameplayServerConfiguration configuration);
void wire_room_close(struct WireRoomHandle handle);
struct String wire_room_get_managerId(struct WireRoomHandle handle);
struct GameplayServerConfiguration wire_room_get_configuration(struct WireRoomHandle handle);
struct PacketContext wire_room_get_protocol(struct WireRoomHandle handle);
struct IPEndPoint wire_block_get_endpoint(struct WireBlockHandle block, _Bool ipv4);

struct NetSession *TEMPwire_room_resolve_session(struct WireRoomHandle handle, struct SS addr, struct String secret, struct String userId, struct ExString userName, struct PacketContext version);
struct NetContext *TEMPwire_block_get_net(struct WireBlockHandle block);
