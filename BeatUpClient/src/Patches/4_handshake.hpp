#pragma once
extern "C" {
	#define restrict
	#include "../packets.h"
	#undef restrict
}
#include <GlobalNamespace/LobbyGameStateController.hpp>
#include <LiteNetLib/NetConstants.hpp>

namespace BeatUpClient {
	extern ServerConnectInfo connectInfo;
}

static constexpr const ServerConnectInfo ServerConnectInfo_Default = {
	.windowSize = LiteNetLib::NetConstants::DefaultWindowSize,
	.countdownDuration = uint8_t(GlobalNamespace::LobbyGameStateController::kShortTimerSeconds * 4),
};
