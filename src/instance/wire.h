#pragma once
#include <stdint.h>

struct WireRoomHandle {
	uint32_t host:18;
	uint16_t room:14;
};
static const struct WireRoomHandle WIRE_HANDLE_INVALID = {0x3ffff, 0x3fff};
static const uint32_t WIRE_HOST_INVALID = WIRE_HANDLE_INVALID.host;

bool TEMPwire_attach_local(uint16_t capacity);
void TEMPwire_room_close_notify(uint32_t host, uint16_t room);
