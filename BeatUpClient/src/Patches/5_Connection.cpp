#include "../global.hpp"
#include "4_handshake.hpp"
#include <GlobalNamespace/BeatmapCharacteristicSO.hpp>
#include <GlobalNamespace/GameLiftConnectionManager.hpp>
#include <GlobalNamespace/LevelSelectionNavigationController.hpp>
#include <GlobalNamespace/MasterServerConnectionManager.hpp>
#include <LiteNetLib/NetPacket.hpp>
#include <LiteNetLib/NetPeer.hpp>
#include <LiteNetLib/ReliableChannel.hpp>
#include <Polyglot/Localization.hpp>
#include <System/Array.hpp>
using namespace BeatUpClient;

static bool enableCustomLevels = false;
void HandleConnectToServerSuccess(bool notGameLift, GlobalNamespace::BeatmapLevelSelectionMask selectionMask, GlobalNamespace::GameplayServerConfiguration configuration) {
	logger->info("ConnectionManager_HandleConnectToServerSuccess()");
	enableCustomLevels = notGameLift && selectionMask.songPacks.Contains(StringW("custom_levelpack_CustomLevels"));
	// playerData.Init(configuration.maxPlayerCount); // TODO: stub
	// lobbyDifficultyPanel.Clear(); // TODO: stub
	connectInfo = ServerConnectInfo_Default;
	// GameObject_SetActive(infoText, false); // TODO: stub
	logger->info("ConnectionManager_HandleConnectToServerSuccess() end");
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MasterServerConnectionManager::HandleConnectToServerSuccess, void, GlobalNamespace::MasterServerConnectionManager *self, ::StringW remoteUserId, ::StringW remoteUserName, ::System::Net::IPEndPoint* remoteEndPoint, ::StringW secret, ::StringW code, ::GlobalNamespace::BeatmapLevelSelectionMask selectionMask, ::GlobalNamespace::GameplayServerConfiguration configuration, ::ArrayW<uint8_t> preMasterSecret, ::ArrayW<uint8_t> myRandom, ::ArrayW<uint8_t> remoteRandom, bool isConnectionOwner, bool isDedicatedServer, ::StringW managerId) {
	HandleConnectToServerSuccess(true, selectionMask, configuration);
	base(self, remoteUserId, remoteUserName, remoteEndPoint, secret, code, selectionMask, configuration, preMasterSecret, myRandom, remoteRandom, isConnectionOwner, isDedicatedServer, managerId);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::GameLiftConnectionManager::HandleConnectToServerSuccess, void, GlobalNamespace::GameLiftConnectionManager *self, ::StringW playerSessionId, ::System::Net::IPEndPoint* remoteEndPoint, ::StringW gameSessionId, ::StringW secret, ::StringW code, ::GlobalNamespace::BeatmapLevelSelectionMask selectionMask, ::GlobalNamespace::GameplayServerConfiguration configuration) {
	HandleConnectToServerSuccess(false, selectionMask, configuration);
	base(self, playerSessionId, remoteEndPoint, gameSessionId, secret, code, selectionMask, configuration);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::LevelSelectionNavigationController::Setup, void, GlobalNamespace::LevelSelectionNavigationController *self, ::GlobalNamespace::SongPackMask songPackMask, ::GlobalNamespace::BeatmapDifficultyMask allowedBeatmapDifficultyMask, ::ArrayW<::GlobalNamespace::BeatmapCharacteristicSO*> notAllowedCharacteristics, bool hidePacksIfOneOrNone, bool hidePracticeButton, ::StringW actionButtonText, ::GlobalNamespace::IBeatmapLevelPack* levelPackToBeSelectedAfterPresent, ::GlobalNamespace::SelectLevelCategoryViewController::LevelCategory startLevelCategory, ::GlobalNamespace::IPreviewBeatmapLevel* beatmapLevelToBeSelectedAfterPresent, bool enableCustomLevels) {
	if(actionButtonText->Equals(Polyglot::Localization::Get("BUTTON_SELECT"))) {
		enableCustomLevels |= ::enableCustomLevels;
		notAllowedCharacteristics = ArrayW<GlobalNamespace::BeatmapCharacteristicSO*>((il2cpp_array_size_t)0);
	}
	base(self, songPackMask, allowedBeatmapDifficultyMask, notAllowedCharacteristics, hidePacksIfOneOrNone, hidePracticeButton, actionButtonText, levelPackToBeSelectedAfterPresent, startLevelCategory, beatmapLevelToBeSelectedAfterPresent, enableCustomLevels);
}

void LiteNetLib_ReliableChannel_ctor(LiteNetLib::ReliableChannel*, LiteNetLib::NetPeer*, bool, uint8_t); // TODO: @ Sc2ad about constructors in codegen
template<>
struct ::il2cpp_utils::il2cpp_type_check::MetadataGetter<&LiteNetLib_ReliableChannel_ctor> {
	static const MethodInfo* get() {
		using funcType = void (*)(LiteNetLib::ReliableChannel*, LiteNetLib::NetPeer*, bool, uint8_t);
		return ::il2cpp_utils::MethodTypeCheck<typename ::il2cpp_utils::InstanceMethodConverter<funcType>::fType>::find("LiteNetLib", "ReliableChannel", ".ctor");
	}
};

BSC_MAKE_HOOK_MATCH(LiteNetLib_ReliableChannel_ctor, void, LiteNetLib::ReliableChannel *self, ::LiteNetLib::NetPeer* peer, bool ordered, uint8_t id) {
	base(self, peer, ordered, id);
	int32_t windowSize = (int32_t)connectInfo.windowSize;
	if(connectInfo.base.protocolId == 0 || windowSize == self->windowSize) {
		logger->info("ReliableChannel_ctor(default)");
		return;
	}
	logger->info("ReliableChannel_ctor(%d)", windowSize);
	self->windowSize = windowSize;
	System::Array::Resize(ByRef(self->pendingPackets), windowSize);
	for(il2cpp_array_size_t i = 1, len = self->pendingPackets.Length(); i < len; ++i)
		self->pendingPackets[i] = self->pendingPackets[0];
	if(self->receivedPackets)
		System::Array::Resize(ByRef(self->receivedPackets), windowSize);
	if(self->earlyReceived)
		System::Array::Resize(ByRef(self->earlyReceived), windowSize);
	il2cpp_utils::RunMethod(self->outgoingAcks, ".ctor", LiteNetLib::PacketProperty(LiteNetLib::PacketProperty::Ack), int((windowSize - 1) / 8 + 2));
	logger->info("ReliableChannel_ctor(%d) end", windowSize);
}
