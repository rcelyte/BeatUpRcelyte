class MainClass {
	static System.Collections.Generic.Dictionary<string, string[]> nonPublicMembers = new System.Collections.Generic.Dictionary<string, string[]>() {
		["BGNet.dll"] = new[] {
			"ConnectedPlayerManager/ConnectedPlayer",
			"ConnectedPlayerManager/ConnectedPlayer::_connection",
			"ConnectedPlayerManager/ConnectedPlayer::_connectionId",
			"ConnectedPlayerManager/ConnectedPlayer::_remoteConnectionId",
			"ConnectedPlayerManager/PlayerConnectedPacket",
			"ConnectedPlayerManager::WriteOne",
		},
		["HMLib.dll"] = new[] {
			"SceneInfo::_sceneName",
		},
		["HMUI.dll"] = new[] {
			"HMUI.DropdownWithTableView::Hide",
			"HMUI.FlowCoordinator::_parentFlowCoordinator",
			"HMUI.FlowCoordinator::_screenSystem",
			"HMUI.FlowCoordinator::DismissFlowCoordinator",
			"HMUI.FlowCoordinator::DismissViewController",
			"HMUI.FlowCoordinator::PresentViewController",
			"HMUI.FlowCoordinator::set_showBackButton",
			"HMUI.FlowCoordinator::SetTitle",
			"HMUI.FlowCoordinator::TopViewControllerWillChange",
			"HMUI.HoverHintController::_isHiding",
			"HMUI.IconSegmentedControl::_container",
			"HMUI.ImageView::_flipGradientColors",
			"HMUI.ImageView::_skew",
			"HMUI.InputFieldView::_placeholderText",
			"HMUI.InputFieldView::_textLengthLimit",
			"HMUI.TextSegmentedControl::_container",
			"HMUI.UIKeyboard::_buttonBinder",
			"HMUI.UIKeyboard::keyWasPressedEvent",
			"HMUI.UIKeyboardKey::_canBeUppercase",
			"HMUI.UIKeyboardKey::_keyCode",
			"HMUI.UIKeyboardKey::_overrideText",
		},
		["LiteNetLib.dll"] = new[] {
			"LiteNetLib.NetConnectAcceptPacket",
			"LiteNetLib.NetPacket",
			"LiteNetLib.ReliableChannel",
		},
		["Main.dll"] = new[] {
			"BeatmapCallbacksController::_startFilterTime",
			"BeatmapCharacteristicSegmentedControlController::_segmentedControl",
			"BeatmapCharacteristicSegmentedControlController::didSelectBeatmapCharacteristicEvent",
			"BeatmapDifficultySegmentedControlController::_difficultySegmentedControl",
			"BeatmapDifficultySegmentedControlController::didSelectDifficultyEvent",
			"ColorsOverrideSettingsPanelController::_editColorSchemeButton",
			"CustomLevelLoader::_defaultAllDirectionsEnvironmentInfo",
			"CustomLevelLoader::_defaultEnvironmentInfo",
			"CustomLevelLoader::_environmentSceneInfoCollection",
			"DropdownSettingsController::_dropdown",
			"EnterPlayerGuestNameViewController::_nameInputFieldView",
			"GameplayCoreSceneSetupData::gameplayModifiers",
			"GameServerPlayerTableCell::_localPlayerBackgroundImage",
			"GameSongController::_beatmapCallbacksController",
			"IncDecSettingsController::_stepValuePicker",
			"JoiningLobbyViewController::_loadingControl",
			"LevelScenesTransitionSetupDataSO::get_gameplayCoreSceneSetupData",
			"MainSystemInit::_networkConfig",
			"MenuTransitionsHelper::_gameScenesManager",
			"MenuTransitionsHelper::_multiplayerLevelScenesTransitionSetupData",
			"MultiplayerController::_playersManager",
			"MultiplayerController::_songStartSyncController",
			"MultiplayerLocalActivePlayerFacade::_gameSongController",
			"MultiplayerLocalActivePlayerInGameMenuViewController::_levelBar",
			"MultiplayerLocalActivePlayerInGameMenuViewController::_localPlayerInGameMenuInitData",
			"MultiplayerLocalActivePlayerInGameMenuViewController::_mainBar",
			"MultiplayerLocalActivePlayerInGameMenuViewController::_resumeButton",
			"MultiplayerModeSelectionFlowCoordinator::_multiplayerModeSelectionViewController",
			"MultiplayerModeSelectionViewController::_customServerEndPointText",
			"MultiplayerModeSelectionViewController::_maintenanceMessageText",
			"MultiplayerStatusModel::_request",
			"NetworkPlayerEntitlementChecker::_additionalContentModel",
			"PCAppInit::TransitionToNextScene",
			"QuickPlaySetupModel::_request",
			"StandardLevelDetailView::_beatmapCharacteristicSegmentedControlController",
			"StandardLevelDetailView::_beatmapDifficultySegmentedControlController",
			"SwitchSettingsController::_toggle",
		},
		["MultiplayerCore.dll"] = new[] {
			"MultiplayerCore.Objects.MpPlayersDataModel::.ctor",
			"MultiplayerCore.Objects.MpPlayersDataModel::_beatmapLevelProvider",
			"MultiplayerCore.Objects.MpPlayersDataModel::_packetSerializer",
		},
		["Polyglot.dll"] = new[] {
			"Polyglot.LocalizationImporter::Import",
			"Polyglot.LocalizedTextComponent`1::localizedComponent",
		},
	};
	class FixedAssemblyResolver : Mono.Cecil.BaseAssemblyResolver {
		public override Mono.Cecil.AssemblyDefinition Resolve(Mono.Cecil.AssemblyNameReference name) {
			if(name.Name == "GameplayCore")
				name.Name = "GamePlayCore";
			return base.Resolve(name);
		}
	}
	public static void Main(string[] args) {
		if(args.Length != 2)
			return;
		FixedAssemblyResolver assemblyResolver = new FixedAssemblyResolver();
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
				foreach(Mono.Cecil.EventDefinition ev in type.Events)
					if(names.Contains(ev.Name))
						ev.Name += "$";
			}
		}
		assembly.MainModule.Write(args[1]);
	}
}
