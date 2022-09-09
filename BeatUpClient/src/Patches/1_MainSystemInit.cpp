#include "../global.hpp"
#include "../Injected.hpp"
#include <GlobalNamespace/BeatmapCharacteristicCollectionSO.hpp>
#include <GlobalNamespace/BeatmapLevelsModel.hpp>
#include <GlobalNamespace/BoolSO.hpp>
#include <GlobalNamespace/CustomLevelLoader.hpp>
#include <GlobalNamespace/CustomNetworkConfig.hpp>
#include <GlobalNamespace/MainSettingsModelSO.hpp>
#include <GlobalNamespace/MainSystemInit.hpp>
#include <GlobalNamespace/MultiplayerSessionManager.hpp>
#include <GlobalNamespace/MultiplayerStatusModel.hpp>
#include <GlobalNamespace/QuickPlaySetupModel.hpp>
using namespace BeatUpClient;

static bool restoreDefault = false;
BSC_MAKE_HOOK_MATCH(GlobalNamespace::MainSettingsModelSO::Load, void, GlobalNamespace::MainSettingsModelSO *self, bool forced) {
	base(self, forced);
	restoreDefault |= !self->useCustomServerEnvironment->value;
	self->useCustomServerEnvironment->set_value(true);
	logger->info("GlobalNamespace::MainSettingsModelSO::Load()");
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MainSettingsModelSO::Save, void, GlobalNamespace::MainSettingsModelSO *self) {
	logger->info("GlobalNamespace::MainSettingsModelSO::Save()");
	self->useCustomServerEnvironment->value = !restoreDefault;
	base(self);
	self->useCustomServerEnvironment->value = true;
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MainSystemInit::InstallBindings, void, GlobalNamespace::MainSystemInit *self, Zenject::DiContainer *container) {
	base(self, container);
	// customServerHostName = ____mainSettingsModel.customServerHostName; // TODO: stub
	// NetworkConfigSetup(__instance._networkConfig); // TODO: stub
	Injected<GlobalNamespace::BeatmapCharacteristicCollectionSO>::Resolve<GlobalNamespace::BeatmapCharacteristicCollectionSO>(container);
	Injected<GlobalNamespace::BeatmapLevelsModel>::Resolve<GlobalNamespace::BeatmapLevelsModel>(container);
	Injected<GlobalNamespace::CustomLevelLoader>::Resolve<GlobalNamespace::CustomLevelLoader>(container);
	Injected<GlobalNamespace::CustomNetworkConfig>::Resolve<GlobalNamespace::INetworkConfig>(container);
	Injected<GlobalNamespace::IMultiplayerStatusModel>::Resolve<GlobalNamespace::IMultiplayerStatusModel>(container);
	Injected<GlobalNamespace::IQuickPlaySetupModel>::Resolve<GlobalNamespace::IQuickPlaySetupModel>(container);
	Injected<GlobalNamespace::MultiplayerSessionManager>::Resolve<GlobalNamespace::IMultiplayerSessionManager>(container)->SetLocalPlayerState("modded", true);
	// Net.Setup(Injected<GlobalNamespace::IMenuRpcManager>::Resolve<GlobalNamespace::IMenuRpcManager>(container)); // TODO: stub
}
