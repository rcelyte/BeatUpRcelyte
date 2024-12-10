using System.IO;
using Mono.Cecil;

if(args.Length < 1) {
	System.Console.WriteLine("Usage: MakeThingsPublic.exe <game folder>");
	System.Environment.Exit(1);
}

string managed = Path.Combine("Beat Saber_Data", "Managed");
var refs = new (string? path, string name, string[]? overrides)[] {
	(managed, "BGNetCore.dll", new[] {
		"ClientCertificateValidator::ValidateCertificateChainInternal",
		"ConnectedPlayerManager/ConnectedPlayer",
		"ConnectedPlayerManager/PlayerConnectedPacket",
		"ConnectedPlayerManager::kMaxUnreliableMessageLength",
		"ConnectedPlayerManager::RemovePlayer",
		"ConnectedPlayerManager::Write",
		"ConnectedPlayerManager::WriteOne",
		"GameLiftConnectionManager::HandleConnectToServerSuccess",
		"LiteNetLibConnectionManager/NetPeerConnection",
		"LiteNetLibConnectionManager::GetConnectionMessage",
		"MasterServerConnectionManager::HandleConnectToServerSuccess",
		"MenuRpcManager::_multiplayerSessionManager",
		"MenuRpcManager::InvokeSetSelectedBeatmap",
		"MultiplayerSessionManager::_packetSerializer",
		"NetworkPacketSerializer`2::_messsageHandlers",
		"NetworkPacketSerializer`2::_typeRegistry",
	}),
	(managed, "HMLib.dll", null),
	(managed, "BeatSaber.ViewSystem.dll", new[] {
		"HMUI.FlowCoordinator::_parentFlowCoordinator",
		"HMUI.FlowCoordinator::_screenSystem",
		"HMUI.FlowCoordinator::DismissFlowCoordinator",
		"HMUI.FlowCoordinator::DismissViewController",
		"HMUI.FlowCoordinator::PresentViewController",
		"HMUI.FlowCoordinator::ProvideInitialViewControllers",
		"HMUI.FlowCoordinator::ReplaceTopViewController",
		"HMUI.FlowCoordinator::set_showBackButton",
		"HMUI.FlowCoordinator::SetTitle",
		"HMUI.FlowCoordinator::TopViewControllerWillChange",
		"HMUI.ImageView::_flipGradientColors",
		"HMUI.ImageView::_skew",
		"HMUI.ModalView::_dismissPanelAnimation",
	}),
	(managed, "HMUI.dll", new[] {
		"HMUI.ButtonSpriteSwap::_disabledStateSprite",
		"HMUI.ButtonSpriteSwap::_highlightStateSprite",
		"HMUI.ButtonSpriteSwap::_normalStateSprite",
		"HMUI.ButtonSpriteSwap::_pressedStateSprite",
		"HMUI.DropdownWithTableView::_modalView",
		"HMUI.DropdownWithTableView::Hide",
		"HMUI.HoverHint::_hoverHintController",
		"HMUI.HoverHintController::_isHiding",
		"HMUI.HoverHintController::SetupAndShowHintPanel",
		"HMUI.InputFieldView::_placeholderText",
		"HMUI.InputFieldView::_textLengthLimit",
		"HMUI.InputFieldView::UpdateClearButton",
		"HMUI.PanelAnimationSO::_duration",
		"HMUI.SimpleTextDropdown::_text",
		"HMUI.UIKeyboard::_buttonBinder",
		"HMUI.UIKeyboard::keyWasPressedEvent",
		"HMUI.UIKeyboardKey::_canBeUppercase",
		"HMUI.UIKeyboardKey::_keyCode",
		"HMUI.UIKeyboardKey::_overrideText",
		"HMUI.UIKeyboardKey::Awake",
	}),
	(managed, "LiteNetLib.dll", new[] {
		"LiteNetLib.NetConnectAcceptPacket",
		"LiteNetLib.NetPacket",
		"LiteNetLib.PacketProperty",
		"LiteNetLib.ReliableChannel",
		"LiteNetLib.ReliableChannel/PendingPacket",
		"LiteNetLib.Utils.NetDataWriter::_position",
	}),
	(managed, "Main.dll", new[] {
		"BeatmapCallbacksController::_startFilterTime",
		"BeatmapCharacteristicSegmentedControlController::_segmentedControl",
		"BeatmapCharacteristicSegmentedControlController::didSelectBeatmapCharacteristicEvent",
		"BeatmapDifficultySegmentedControlController::_difficultySegmentedControl",
		"BeatmapDifficultySegmentedControlController::didSelectDifficultyEvent",
		"ColorsOverrideSettingsPanelController::_editColorSchemeButton",
		"CustomLevelLoader::_defaultAllDirectionsEnvironmentInfo",
		"CustomLevelLoader::_defaultEnvironmentInfo",
		"CustomLevelLoader::_environmentSceneInfoCollection",
		"CustomPreviewBeatmapLevel::_coverImage",
		"DropdownSettingsController::_dropdown",
		"DropdownSettingsController::_idx",
		"EditColorSchemeController::_closeButton",
		"EnterPlayerGuestNameViewController::_nameInputFieldView",
		"GameplayCoreSceneSetupData::gameplayModifiers",
		"GameServerPlayerTableCell::_localPlayerBackgroundImage",
		"GameSongController::_beatmapCallbacksController",
		"JoiningLobbyViewController::_loadingControl",
		"LevelScenesTransitionSetupDataSO::get_gameplayCoreSceneSetupData",
		"MainFlowCoordinator::_simpleDialogPromptViewController",
		"MainFlowCoordinator::HandleMainMenuViewControllerDidFinish",
		"MainFlowCoordinator::HandleMultiplayerModeSelectionFlowCoordinatorDidFinish",
		"MainFlowCoordinator::PresentMultiplayerModeSelectionFlowCoordinatorWithDisclaimerAndAvatarCreator",
		"MainSystemInit::_mainSettingsModel",
		"MenuTransitionsHelper::_gameScenesManager",
		"MenuTransitionsHelper::_multiplayerLevelScenesTransitionSetupData",
		"MultiplayerConnectedPlayerInstaller::_connectedPlayer",
		"MultiplayerConnectedPlayerInstaller::_sceneSetupData",
		"MultiplayerController::_playersManager",
		"MultiplayerController::_songStartSyncController",
		"MultiplayerController::GetCurrentSongTime",
		"MultiplayerController::GetSongStartSyncTime",
		"MultiplayerLevelLoader::_getBeatmapCancellationTokenSource",
		"MultiplayerLevelLoader::_loaderState",
		"MultiplayerLocalActivePlayerFacade::_gameSongController",
		"MultiplayerLocalActivePlayerInGameMenuViewController::_levelBar",
		"MultiplayerLocalActivePlayerInGameMenuViewController::_localPlayerInGameMenuInitData",
		"MultiplayerLocalActivePlayerInGameMenuViewController::_mainBar",
		"MultiplayerLocalActivePlayerInGameMenuViewController::_resumeButton",
		"MultiplayerLocalActivePlayerInGameMenuViewController::Start",
		"MultiplayerModeSelectionFlowCoordinator::_multiplayerModeSelectionViewController",
		"MultiplayerModeSelectionFlowCoordinator::PresentMasterServerUnavailableErrorDialog",
		"MultiplayerModeSelectionViewController::_customServerEndPointText",
		"MultiplayerModeSelectionViewController::_maintenanceMessageText",
		"StandardLevelDetailView::_beatmapCharacteristicSegmentedControlController",
		"StandardLevelDetailView::_beatmapDifficultySegmentedControlController",
	}),
	(managed, "mscorlib.dll", new[] {
		"System.Runtime.CompilerServices.Unsafe",
		"System.Security.Cryptography.HashAlgorithm::HashCore",
		"System.Security.Cryptography.HashAlgorithm::HashFinal",
	}),
	(null, "MultiplayerCore.dll", new[] {
		"MultiplayerCore.Beatmaps.Abstractions.MpBeatmapLevel::set_previewDifficultyBeatmapSets",
		"MultiplayerCore.Networking.MpPacketSerializer::registeredTypes",
		"MultiplayerCore.Objects.MpEntitlementChecker",
		"MultiplayerCore.Objects.MpPlayersDataModel",
		"MultiplayerCore.Patchers.CustomLevelsPatcher",
		"MultiplayerCore.Patchers.ModeSelectionPatcher",
		"MultiplayerCore.Patches.DataModelBinderPatch",
		"MultiplayerCore.Patches.DataModelBinderPatch::_playersDataModelMethod",
		"MultiplayerCore.Patches.OverridePatches.PlayersDataModelOverride",
		"MultiplayerCore.Plugin",
		"MultiplayerCore.UI.MpPerPlayerUI",
		"MultiplayerCore.UI.MpPerPlayerUI::segmentVert",
	}),
	(managed, "BGLib.Polyglot.dll", new[] {
		"BGLib.Polyglot.LocalizationImporter::Import",
		"BGLib.Polyglot.LocalizationImporter::Initialize",
		"BGLib.Polyglot.LocalizedTextComponent`1::localizedComponent",
	}),
	(null, "SiraUtil.dll", new[] {
		"SiraUtil.Affinity.Harmony.HarmonyAffinityPatcher",
	}),
	(managed, "Zenject.dll", new[] {
		"Zenject.Context::InstallInstallers",
		"Zenject.MonoInstallerBase::get_Container",
	}),
	("Libs", "0Harmony.dll", null),
	("Libs", "Hive.Versioning.dll", null),
	("Libs", "Mono.Cecil.dll", null),
	("Libs", "MonoMod.RuntimeDetour.dll", null),
	("Libs", "MonoMod.Utils.dll", null),
	("Libs", "Newtonsoft.Json.dll", null),
	(null, "SongCore.dll", null),
	(managed, "AdditionalContentModel.Interfaces.dll", null),
	(managed, "BeatmapCore.dll", null),
	(managed, "BeatSaber.GameSettings.dll", null),
	(managed, "BeatSaber.Settings.dll", null),
	(managed, "BeatSaber.Init.dll", null),
	(managed, "BGLib.AppFlow.dll", new[] {
		"SceneInfo::_sceneName",
	}),
	(managed, "BGLib.DotnetExtension.dll", null),
	(managed, "BGLib.SaveDataCore.dll", null),
	(managed, "BGLib.UnityExtension.dll", null),
	(managed, "Colors.dll", null),
	(managed, "GameInit.dll", new[] {
		"MainSettingsAsyncLoader::_mainSettingsHandler",
		"MainSettingsAsyncLoader::_networkConfig",
		"PCAppInit::TransitionToNextScene",
	}),
	(managed, "GameplayCore.dll", null),
	(managed, "Ignorance.dll", null),
	(managed, "Interactable.dll", null),
	(managed, "IPA.Loader.dll", null),
	(managed, "Menu.CommonLib.dll", new[] {
		"IncDecSettingsController::_stepValuePicker",
		"SwitchSettingsController::_toggle",
	}),
	(managed, "Networking.dll", new[] {
		"NetworkConfigSO::_discoveryPort",
		"NetworkConfigSO::_forceGameLift",
		"NetworkConfigSO::_masterServerHostName",
		"NetworkConfigSO::_masterServerPort",
		"NetworkConfigSO::_maxPartySize",
		"NetworkConfigSO::_multiplayerPort",
		"NetworkConfigSO::_multiplayerStatusUrl",
		"NetworkConfigSO::_partyPort",
		"NetworkConfigSO::_quickPlaySetupUrl",
		"NetworkConfigSO::_serviceEnvironment",
		"PlatformAuthenticationTokenProvider::_platform",
		"PlatformAuthenticationTokenProvider::_userId",
		"PlatformAuthenticationTokenProvider::_userName",
	}),
	(managed, "PlatformUserModel.dll", new[] {
		"PlatformAuthenticationTokenProvider::_platform",
		"PlatformAuthenticationTokenProvider::_userId",
		"PlatformAuthenticationTokenProvider::_userName",
	}),
	(managed, "SegmentedControl.dll", new[] {
		"HMUI.SegmentedControl::_container",
	}),
	(managed, "System.Net.Http.dll", null),
	(managed, "Unity.ResourceManager.dll", new[] {
		"MonoBehaviourCallbackHooks",
	}),
	(managed, "Unity.TextMeshPro.dll", null),
	(managed, "UnityEngine.AssetBundleModule.dll", null),
	(managed, "UnityEngine.AudioModule.dll", null),
	(managed, "UnityEngine.CoreModule.dll", null),
	(managed, "UnityEngine.ImageConversionModule.dll", null),
	(managed, "UnityEngine.JSONSerializeModule.dll", null),
	(managed, "UnityEngine.UI.dll", null),
	(managed, "UnityEngine.UIModule.dll", null),
	(managed, "Networking.NetworkPlayerEntitlementsChecker.dll", new[] {
		"NetworkPlayerEntitlementChecker::_additionalContentModel",
		"NetworkPlayerEntitlementChecker::GetEntitlementStatus",
		"NetworkPlayerEntitlementChecker::HandleGetIsEntitledToLevel",
	}),
	(managed, "UnityEngine.UnityWebRequestAudioModule.dll", null),
	(managed, "UnityEngine.UnityWebRequestModule.dll", null),
	(managed, "VRUI.dll", null),
	(managed, "Zenject-usage.dll", null),
	(managed, "MediaLoader.dll", null),
	(managed, "DataModels.dll", new[] {
		"BeatmapKey::beatmapCharacteristic",
		"BeatmapLevelsModel::_allLoadedBeatmapLevelsRepository",
		"BeatmapLevelsModel::_entitlements",
		"BeatmapLevelsRepository::_idToBeatmapLevel",
		"FileDifficultyBeatmap::_beatmapPath",
		"FileDifficultyBeatmap::_lightshowPath",
		"FileSystemBeatmapLevelData::_audioDataPath",
		"FileSystemBeatmapLevelData::_difficultyBeatmaps",
		"LobbyPlayersDataModel::_multiplayerSessionManager",
		"LobbyPlayersDataModel::HandleMenuRpcManagerGetRecommendedBeatmap",
		"LobbyPlayersDataModel::HandleMultiplayerSessionManagerPlayerConnected",
		"MultiplayerStatusModel::_request",
		"QuickPlaySetupModel::_request",
	}),

	(managed, "netstandard.dll", null), // TODO: Required by `Hive.Versioning.VersionRange`; remove once beta version check is gone
};

string outDir = Path.Combine(".obj", "Refs"), beatSaberDir = string.Join(" ", args);
foreach(var lib in refs) {
	FileInfo target = new(Path.Combine(Path.Combine(outDir, Path.GetFileName(lib.path ?? "Plugins")), lib.name));
	FileInfo source = new(Path.Combine((lib.path != null) ? Path.Combine(beatSaberDir, lib.path) : "thirdparty", lib.name));
	if(!source.Exists) {
		System.Console.WriteLine("File not found: " + source.FullName);
		System.Environment.Exit(1);
	}
	if(target.LastWriteTime >= source.LastWriteTime)
		continue;
	Directory.CreateDirectory(target.DirectoryName);
	if(lib.overrides == null) {
		source.CopyTo(target.FullName, true);
		continue;
	}
	System.Console.WriteLine("Processing " + source.Name);
	DefaultAssemblyResolver resolver = new();
	resolver.AddSearchDirectory(source.DirectoryName);
	AssemblyDefinition assembly = AssemblyDefinition.ReadAssembly(source.FullName, new() {AssemblyResolver = resolver});
	System.Linq.ILookup<string, string> typeLookup = System.Linq.Enumerable.ToLookup(lib.overrides, e => e.Split(':')[0], e => $"{e}::".Split(':')[2]);
	foreach(TypeDefinition type in assembly.MainModule.GetTypes()) {
		System.Collections.Generic.HashSet<string> names = System.Linq.Enumerable.ToHashSet(typeLookup[type.FullName]);
		if(names.Contains(""))
			type.IsPublic = true;
		foreach(MethodDefinition method in type.Methods)
			if(names.Contains(method.Name))
				method.IsPublic = true;
		foreach(FieldDefinition field in type.Fields) {
			if(names.Contains(field.Name)) {
				field.IsPublic = true;
				field.IsInitOnly = false;
			}
		}
		foreach(EventDefinition ev in type.Events)
			if(names.Contains(ev.Name))
				ev.Name += "$";
	}
	assembly.MainModule.Write(target.FullName);
}
