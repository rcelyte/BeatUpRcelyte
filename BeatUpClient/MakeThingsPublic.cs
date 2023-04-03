using System.IO;
using Mono.Cecil;

if(args.Length < 1) {
	System.Console.WriteLine("Usage: MakeThingsPublic.exe <game folder>");
	System.Environment.Exit(1);
}

string managed = Path.Combine("Beat Saber_Data", "Managed");
var refs = new (string path, string name, string[]? overrides)[] {
	(managed, "BGNet.dll", new[] {
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
		"MultiplayerSessionManager::_packetSerializer",
		"NetworkPacketSerializer`2::_messsageHandlers",
		"NetworkPacketSerializer`2::_typeRegistry",
	}),
	(managed, "HMLib.dll", new[] {
		"SceneInfo::_sceneName",
	}),
	(managed, "HMUI.dll", new[] {
		"HMUI.ButtonSpriteSwap::_disabledStateSprite",
		"HMUI.ButtonSpriteSwap::_highlightStateSprite",
		"HMUI.ButtonSpriteSwap::_normalStateSprite",
		"HMUI.ButtonSpriteSwap::_pressedStateSprite",
		"HMUI.DropdownWithTableView::_modalView",
		"HMUI.DropdownWithTableView::Hide",
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
		"HMUI.HoverHintController::_isHiding",
		"HMUI.IconSegmentedControl::_container",
		"HMUI.ImageView::_flipGradientColors",
		"HMUI.ImageView::_skew",
		"HMUI.InputFieldView::_placeholderText",
		"HMUI.InputFieldView::_textLengthLimit",
		"HMUI.ModalView::_dismissPanelAnimation",
		"HMUI.PanelAnimationSO::_duration",
		"HMUI.SimpleTextDropdown::_text",
		"HMUI.TextSegmentedControl::_container",
		"HMUI.UIKeyboard::_buttonBinder",
		"HMUI.UIKeyboard::keyWasPressedEvent",
		"HMUI.UIKeyboardKey::_canBeUppercase",
		"HMUI.UIKeyboardKey::_keyCode",
		"HMUI.UIKeyboardKey::_overrideText",
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
		"BeatmapLevelsModel::_loadedPreviewBeatmapLevels",
		"ColorsOverrideSettingsPanelController::_editColorSchemeButton",
		"CustomLevelLoader::_defaultAllDirectionsEnvironmentInfo",
		"CustomLevelLoader::_defaultEnvironmentInfo",
		"CustomLevelLoader::_environmentSceneInfoCollection",
		"DropdownSettingsController::_dropdown",
		"DropdownSettingsController::_idx",
		"EditColorSchemeController::_closeButton",
		"EnterPlayerGuestNameViewController::_nameInputFieldView",
		"GameplayCoreSceneSetupData::gameplayModifiers",
		"GameServerPlayerTableCell::_localPlayerBackgroundImage",
		"GameSongController::_beatmapCallbacksController",
		"IncDecSettingsController::_stepValuePicker",
		"JoiningLobbyViewController::_loadingControl",
		"LevelScenesTransitionSetupDataSO::get_gameplayCoreSceneSetupData",
		"LobbyPlayersDataModel::_multiplayerSessionManager",
		"MainFlowCoordinator::_simpleDialogPromptViewController",
		"MainSystemInit::_mainSettingsModel",
		"MainSystemInit::_networkConfig",
		"MenuTransitionsHelper::_gameScenesManager",
		"MenuTransitionsHelper::_multiplayerLevelScenesTransitionSetupData",
		"MultiplayerController::_playersManager",
		"MultiplayerController::_songStartSyncController",
		"MultiplayerLevelLoader::_getBeatmapCancellationTokenSource",
		"MultiplayerLocalActivePlayerFacade::_gameSongController",
		"MultiplayerLocalActivePlayerInGameMenuViewController::_levelBar",
		"MultiplayerLocalActivePlayerInGameMenuViewController::_localPlayerInGameMenuInitData",
		"MultiplayerLocalActivePlayerInGameMenuViewController::_mainBar",
		"MultiplayerLocalActivePlayerInGameMenuViewController::_resumeButton",
		"MultiplayerModeSelectionFlowCoordinator::_multiplayerModeSelectionViewController",
		"MultiplayerModeSelectionViewController::_customServerEndPointText",
		"MultiplayerModeSelectionViewController::_maintenanceMessageText",
		"MultiplayerStatusModel::_request",
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
		"NetworkPlayerEntitlementChecker::_additionalContentModel",
		"PCAppInit::TransitionToNextScene",
		"PlatformAuthenticationTokenProvider::_platform",
		"PlatformAuthenticationTokenProvider::_userId",
		"PlatformAuthenticationTokenProvider::_userName",
		"QuickPlaySetupModel::_request",
		"StandardLevelDetailView::_beatmapCharacteristicSegmentedControlController",
		"StandardLevelDetailView::_beatmapDifficultySegmentedControlController",
		"SwitchSettingsController::_toggle",
	}),
	(managed, "mscorlib.dll", new[] {
		"System.Runtime.CompilerServices.Unsafe",
		"System.Security.Cryptography.HashAlgorithm::HashCore",
		"System.Security.Cryptography.HashAlgorithm::HashFinal",
	}),
	("Plugins", "MultiplayerCore.dll", new[] {
		"MultiplayerCore.Beatmaps.Abstractions.MpBeatmapLevel::set_previewDifficultyBeatmapSets",
		"MultiplayerCore.Networking.MpPacketSerializer::registeredTypes",
		"MultiplayerCore.Objects.MpEntitlementChecker",
		"MultiplayerCore.Objects.MpPlayersDataModel",
		"MultiplayerCore.Patchers.CustomLevelsPatcher",
		"MultiplayerCore.Patchers.ModeSelectionPatcher",
		"MultiplayerCore.Patches.DataModelBinderPatch",
		"MultiplayerCore.Patches.DataModelBinderPatch::_playersDataModelMethod",
		"MultiplayerCore.Plugin",
	}),
	(managed, "Polyglot.dll", new[] {
		"Polyglot.LocalizationImporter::Import",
		"Polyglot.LocalizedTextComponent`1::localizedComponent",
	}),
	("Plugins", "SiraUtil.dll", new[] {
		"SiraUtil.Affinity.Harmony.HarmonyAffinityPatcher",
	}),
	(managed, "System.IO.Compression.dll", new[] {
		"System.IO.Compression.ZipArchiveEntry::get_UncompressedData",
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
	("Plugins", "SongCore.dll", null),
	(managed, "BeatmapCore.dll", null),
	(managed, "Colors.dll", null),
	(managed, "GameplayCore.dll", null),
	(managed, "IPA.Loader.dll", null),
	(managed, "Unity.TextMeshPro.dll", null),
	(managed, "UnityEngine.AssetBundleModule.dll", null),
	(managed, "UnityEngine.AudioModule.dll", null),
	(managed, "UnityEngine.CoreModule.dll", null),
	(managed, "UnityEngine.ImageConversionModule.dll", null),
	(managed, "UnityEngine.UI.dll", null),
	(managed, "UnityEngine.UIModule.dll", null),
	(managed, "UnityEngine.UnityWebRequestAudioModule.dll", null),
	(managed, "UnityEngine.UnityWebRequestModule.dll", null),
	(managed, "VRUI.dll", null),
	(managed, "Zenject-usage.dll", null),

	(managed, "netstandard.dll", null), // TODO: Required by `Hive.Versioning.VersionRange`; remove once beta version check is gone
};

string outDir = Path.Combine(".obj", "Refs"), beatSaberDir = string.Join(" ", args);
foreach(var lib in refs) {
	FileInfo target = new(Path.Combine(Path.Combine(outDir, Path.GetFileName(lib.path)), lib.name));
	FileInfo source = new(Path.Combine(Path.Combine(beatSaberDir, lib.path), lib.name));
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
