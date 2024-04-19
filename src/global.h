#pragma once
#include "../common/global.h"
#include "log.h"
#include <stdint.h>

#ifdef WINDOWS
typedef uintptr_t NetSocket;
static const NetSocket NetSocket_Invalid = ~(NetSocket)0;
#else
typedef int32_t NetSocket;
static const NetSocket NetSocket_Invalid = -1;
#endif

struct DataView {
	void *data;
	size_t length;
};
