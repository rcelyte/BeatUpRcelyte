class MainClass {
	static System.Collections.Generic.Dictionary<string, string[]> nonPublicMembers = new System.Collections.Generic.Dictionary<string, string[]>() {
		["BGNet.dll"] = new[] {
			"ConnectedPlayerManager/ConnectedPlayer::_connection",
			"ConnectedPlayerManager/ConnectedPlayer::_connectionId",
			"ConnectedPlayerManager/ConnectedPlayer::_remoteConnectionId",
			"ConnectedPlayerManager/PlayerConnectedPacket",
			"ConnectedPlayerManager::WriteOne",
		 	"ConnectedPlayerManager/ConnectedPlayer",
		},
		["HMLib.dll"] = new[] {
			"SceneInfo::_sceneName",
		},
		["HMUI.dll"] = new[] {
			"HMUI.DropdownWithTableView::Hide",
			"HMUI.FlowCoordinator::_screenSystem",
			"HMUI.FlowCoordinator::DismissFlowCoordinator",
			"HMUI.FlowCoordinator::DismissViewController",
			"HMUI.FlowCoordinator::PresentViewController",
			"HMUI.FlowCoordinator::ReplaceTopViewController",
			"HMUI.FlowCoordinator::ReplaceTopViewController",
			"HMUI.FlowCoordinator::SetTitle",
			"HMUI.IconSegmentedControl::_container",
			"HMUI.ImageView::_flipGradientColors",
			"HMUI.ImageView::_skew",
			"HMUI.InputFieldView::_placeholderText",
			"HMUI.InputFieldView::_textLengthLimit",
			"HMUI.InputFieldView::UpdateClearButton",
			"HMUI.TextSegmentedControl::_container",
			"HMUI.UIKeyboard::_buttonBinder",
			"HMUI.UIKeyboardKey::_canBeUppercase",
			"HMUI.UIKeyboardKey::_keyCode",
			"HMUI.UIKeyboardKey::_overrideText",
			"HMUI.UIKeyboardKey::Awake",
		},
		["LiteNetLib.dll"] = new[] {
			"LiteNetLib.NetConnectAcceptPacket",
			"LiteNetLib.NetPacket",
			"LiteNetLib.ReliableChannel",
		},
		["Main.dll"] = new[] {
			"BeatmapCallbacksController::_startFilterTime",
			"BeatmapCharacteristicSegmentedControlController::_segmentedControl",
			"BeatmapDifficultySegmentedControlController::_difficultySegmentedControl",
			"ColorsOverrideSettingsPanelController::_editColorSchemeButton",
			"CustomLevelLoader::_defaultAllDirectionsEnvironmentInfo",
			"CustomLevelLoader::_defaultEnvironmentInfo",
			"CustomLevelLoader::_environmentSceneInfoCollection",
			"DropdownSettingsController::_dropdown",
			"EnterPlayerGuestNameViewController::_nameInputFieldView",
			"GameServerPlayerTableCell::_localPlayerBackgroundImage",
			"GameServerPlayerTableCell::Awake",
			"GameSongController::_beatmapCallbacksController",
			"IncDecSettingsController::_stepValuePicker",
			"JoiningLobbyViewController::_text",
			"LevelScenesTransitionSetupDataSO::get_gameplayCoreSceneSetupData",
			"LobbyPlayersDataModel::HandleMenuRpcManagerGetRecommendedBeatmap",
			"MainSystemInit::_networkConfig",
			"MenuTransitionsHelper::_gameScenesManager",
			"MenuTransitionsHelper::_multiplayerLevelScenesTransitionSetupData",
			"MultiplayerController::_playersManager",
			"MultiplayerController::_songStartSyncController",
			"MultiplayerController::GetCurrentSongTime",
			"MultiplayerController::GetSongStartSyncTime",
			"MultiplayerLobbyAvatarManager::AddPlayer",
			"MultiplayerLocalActivePlayerFacade::_gameSongController",
			"MultiplayerLocalActivePlayerInGameMenuViewController::_levelBar",
			"MultiplayerLocalActivePlayerInGameMenuViewController::_localPlayerInGameMenuInitData",
			"MultiplayerLocalActivePlayerInGameMenuViewController::_mainBar",
			"MultiplayerLocalActivePlayerInGameMenuViewController::_resumeButton",
			"MultiplayerLocalActivePlayerInGameMenuViewController::Start",
			"MultiplayerModeSelectionFlowCoordinator::PresentMasterServerUnavailableErrorDialog",
			"MultiplayerModeSelectionFlowCoordinator::ProcessDeeplinkingToLobby",
			"MultiplayerModeSelectionViewController::_customServerEndPointText",
			"MultiplayerModeSelectionViewController::_maintenanceMessageText",
			"MultiplayerStatusModel::_request",
			"NetworkPlayerEntitlementChecker::_additionalContentModel",
			"NetworkPlayerEntitlementChecker::_rpcManager",
			"PCAppInit::TransitionToNextScene",
			"QuickPlaySetupModel::_request",
			"StandardLevelDetailView::_beatmapCharacteristicSegmentedControlController",
			"StandardLevelDetailView::_beatmapDifficultySegmentedControlController",
			"SwitchSettingsController::_toggle",
		},
		["MultiplayerCore.dll"] = new[] {
			"MultiplayerCore.Objects.MpLevelLoader::.ctor",
			"MultiplayerCore.Objects.MpPlayersDataModel::.ctor",
			"MultiplayerCore.Objects.MpPlayersDataModel::_beatmapLevelProvider",
			"MultiplayerCore.Objects.MpPlayersDataModel::_packetSerializer",
		},
		["Polyglot.dll"] = new[] {
			"Polyglot.LocalizationImporter::Import",
		},
	};
	public static void Main(string[] args) {
		if(args.Length != 2)
			return;
		Mono.Cecil.DefaultAssemblyResolver assemblyResolver = new Mono.Cecil.DefaultAssemblyResolver();
		assemblyResolver.AddSearchDirectory(System.IO.Path.GetDirectoryName(args[0]));
		Mono.Cecil.AssemblyDefinition assembly = Mono.Cecil.AssemblyDefinition.ReadAssembly(args[0], new Mono.Cecil.ReaderParameters {AssemblyResolver = assemblyResolver});
		if(nonPublicMembers.TryGetValue(assembly.MainModule.Name, out string[] overrides)) {
			System.Linq.ILookup<string, string> typeLookup = System.Linq.Enumerable.ToLookup(overrides, e => e.Split(':')[0], e => $"{e}::".Split(':')[2]);
			foreach(Mono.Cecil.TypeDefinition type in assembly.MainModule.GetTypes()) {
				System.Collections.Generic.HashSet<string> names = System.Linq.Enumerable.ToHashSet(typeLookup[type.FullName]);
				if(names.Contains(""))
					type.IsPublic = true;
				foreach(Mono.Cecil.MethodDefinition method in type.Methods)
					if(names.Contains(method.Name))
						method.IsPublic = true;
				foreach(Mono.Cecil.FieldDefinition field in type.Fields) {
					if(names.Contains(field.Name)) {
						field.IsPublic = true;
						field.IsInitOnly = false;
					}
				}
			}
		}
		assembly.MainModule.Write(args[1]);
	}
}
