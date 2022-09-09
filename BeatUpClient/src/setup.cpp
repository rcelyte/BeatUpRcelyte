#include "global.hpp"
#include "Error.hpp"
#include <GlobalNamespace/NetworkConstants.hpp>
#include <UnityEngine/SceneManagement/SceneManager.hpp>
#define DL_EXPORT __attribute__((visibility("default")))
using namespace BeatUpClient;

Logger *BeatUpClient::logger = NULL;
GlobalNamespace::MainFlowCoordinator *BeatUpClient::mainFlowCoordinator = NULL;
UnityEngine::Sprite *BeatUpClient::altCreateButtonSprites[4] = {};

static void OnSceneLoaded(UnityEngine::SceneManagement::Scene scene, UnityEngine::SceneManagement::LoadSceneMode mode) {
	if(to_utf8(csstrtostr(scene.get_name())) != "MainMenu")
		return;
	logger->info("load MainMenu");
	mainFlowCoordinator = CRASH_UNLESS(FindResourceOfType<GlobalNamespace::MainFlowCoordinator>());
	// if(Resolve<CustomNetworkConfig>() != null)
	// 	SelectorSetup();
	// LobbyUISetup();
}

extern "C" DL_EXPORT void setup(ModInfo& info) {
	info.id = "BeatUpClient";
	info.version = VERSION;
	// new(&config) Configuration(info);
	logger = new Logger(info, LoggerOptions(false, true));
	// config.Load();
	logger->info("Completed setup!");
}

extern "C" DL_EXPORT void load() {
	logger->info("loading @ %c%c", __TIME__[3], __TIME__[4]);
	il2cpp_functions::Init();

	uint32_t protocolVersion = GlobalNamespace::NetworkConstants::_get_kProtocolVersion();
	if(protocolVersion < 8u) {
		BeatUpClient_Error::Init("Incompatible BeatUpClient Version", "This version of BeatUpClient requires a newer version of Beat Saber.");
		return;
	} else if(protocolVersion > 8u) {
		BeatUpClient_Error::Init("Incompatible BeatUpClient Version", "This version of BeatUpClient requires an older version of Beat Saber.");
		return;
	}
	// string? err = BeatUpClient_Beta.CheckVersion(version);
	logger->info("custom_types::Register::AutoRegister()");
	custom_types::Register::AutoRegister();
	BeatUpClient::ApplyPatches();
	// Polyglot.LocalizationImporter.Import(localization, Polyglot.GoogleDriveDownloadFormat.TSV);
	logger->info("UnityEngine::SceneManagement::SceneManager::add_sceneLoaded()");
	UnityEngine::SceneManagement::SceneManager::add_sceneLoaded(Delegate_Unity(OnSceneLoaded));
	logger->info("DONE LOADING");
	BeatUpClient_Error::Init("TEST HEADER", "TEST MESSAGE");
}
