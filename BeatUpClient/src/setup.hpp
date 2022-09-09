#pragma once
#include "beatsaber-hook/shared/utils/logging.hpp"
#include <GlobalNamespace/MainFlowCoordinator.hpp>
#include <UnityEngine/Sprite.hpp>

namespace BeatUpClient {
	extern Logger *logger;
	extern GlobalNamespace::MainFlowCoordinator *mainFlowCoordinator;
	extern UnityEngine::Sprite *altCreateButtonSprites[4];
}
