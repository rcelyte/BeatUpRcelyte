#include "wire.h"
#include "instance/instance.h"

_Bool wire_request_block(struct WireBlockHandle *block_out, uint16_t notify) {
	fprintf(stderr, "TODO: multiple threads; multiple hosts\n");
	block_out->host = 0;
	block_out->thread = 0;
	return instance_request_block(block_out->thread, &block_out->group, notify);
}

void wire_block_release(struct WireBlockHandle block) {}

_Bool wire_room_open(struct WireRoomHandle handle, struct String managerId, struct GameplayServerConfiguration configuration) {
	if(handle.block.host != 0 || handle.block.thread != 0)
		return 1;
	return instance_room_open(handle.block.thread, handle.block.group, handle.sub, managerId, configuration);
}

void wire_room_close(struct WireRoomHandle handle) {
	instance_room_close(handle.block.thread, handle.block.group, handle.sub);
}

struct String wire_room_get_managerId(struct WireRoomHandle handle) {
	return instance_room_get_managerId(handle.block.thread, handle.block.group, handle.sub);
}

struct GameplayServerConfiguration wire_room_get_configuration(struct WireRoomHandle handle) {
	return instance_room_get_configuration(handle.block.thread, handle.block.group, handle.sub);
}

struct PacketContext wire_room_get_protocol(struct WireRoomHandle handle) {
	return instance_room_get_protocol(handle.block.thread, handle.block.group, handle.sub);
}

struct IPEndPoint wire_block_get_endpoint(struct WireBlockHandle block, _Bool ipv4) {
	return instance_get_endpoint(ipv4);
}

struct NetSession *TEMPwire_room_resolve_session(struct WireRoomHandle handle, struct SS addr, struct String secret, struct String userId, struct String userName, struct PacketContext version) {
	return instance_room_resolve_session(handle.block.thread, handle.block.group, handle.sub, addr, secret, userId, userName, version);
}

struct NetContext *TEMPwire_block_get_net(struct WireBlockHandle block) {
	return instance_get_net(block.thread);
}
