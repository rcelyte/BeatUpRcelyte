class MainClass {
	static string[] overrides = new[] {
		"BeatmapCallbacksController:_startFilterTime",
		"BeatmapCharacteristicSegmentedControlController:_segmentedControl",
		"BeatmapDifficultySegmentedControlController:_difficultySegmentedControl",
		"ColorsOverrideSettingsPanelController:_editColorSchemeButton",
		"ConnectedPlayer",
		"ConnectedPlayer:_connection",
		"ConnectedPlayer:_connectionId",
		"ConnectedPlayer:_remoteConnectionId",
		"ConnectedPlayerManager:WriteOne",
		"CustomLevelLoader:_defaultAllDirectionsEnvironmentInfo",
		"CustomLevelLoader:_defaultEnvironmentInfo",
		"CustomLevelLoader:_environmentSceneInfoCollection",
		"DropdownSettingsController:_dropdown",
		"EnterPlayerGuestNameViewController:_nameInputFieldView",
		"FlowCoordinator:_screenSystem",
		"FlowCoordinator:DismissFlowCoordinator",
		"FlowCoordinator:DismissViewController",
		"FlowCoordinator:PresentViewController",
		"FlowCoordinator:ReplaceTopViewController",
		"FlowCoordinator:SetTitle",
		"GameServerPlayerTableCell:_localPlayerBackgroundImage",
		"GameSongController:_beatmapCallbacksController",
		"IconSegmentedControl:_container",
		"ImageView:_flipGradientColors",
		"ImageView:_skew",
		"IncDecSettingsController:_stepValuePicker",
		"InputFieldView:_placeholderText",
		"InputFieldView:_textLengthLimit",
		"JoiningLobbyViewController:_text",
		"LevelScenesTransitionSetupDataSO:get_gameplayCoreSceneSetupData",
		"LocalizationImporter:Import",
		"MainSystemInit:_networkConfig",
		"MenuTransitionsHelper:_gameScenesManager",
		"MenuTransitionsHelper:_multiplayerLevelScenesTransitionSetupData",
		"MpLevelLoader:.ctor",
		"MpPlayersDataModel:.ctor",
		"MpPlayersDataModel:_beatmapLevelProvider",
		"MpPlayersDataModel:_packetSerializer",
		"MultiplayerController:_playersManager",
		"MultiplayerController:_songStartSyncController",
		"MultiplayerLocalActivePlayerFacade:_gameSongController",
		"MultiplayerLocalActivePlayerInGameMenuViewController:_levelBar",
		"MultiplayerLocalActivePlayerInGameMenuViewController:_localPlayerInGameMenuInitData",
		"MultiplayerLocalActivePlayerInGameMenuViewController:_mainBar",
		"MultiplayerLocalActivePlayerInGameMenuViewController:_resumeButton",
		"MultiplayerModeSelectionViewController:_customServerEndPointText",
		"MultiplayerModeSelectionViewController:_maintenanceMessageText",
		"NetConnectAcceptPacket",
		"NetPacket",
		"NetworkPlayerEntitlementChecker:_additionalContentModel",
		"NetworkPlayerEntitlementChecker:_rpcManager",
		"PlayerConnectedPacket",
		"ReliableChannel",
		"StandardLevelDetailView:_beatmapCharacteristicSegmentedControlController",
		"StandardLevelDetailView:_beatmapDifficultySegmentedControlController",
		"SwitchSettingsController:_toggle",
		"TextSegmentedControl:_container",
		"UIKeyboard:_buttonBinder",
		"UIKeyboardKey:_canBeUppercase",
		"UIKeyboardKey:_keyCode",
		"UIKeyboardKey:_overrideText",
	};
	public static void Main(string[] args) {
		if(args.Length != 2)
			return;
		System.Linq.ILookup<string, string> typeLookup = System.Linq.Enumerable.ToLookup(overrides, e => e.Split(':')[0], e => $"{e}:$".Split(':')[1]);
		Mono.Cecil.DefaultAssemblyResolver assemblyResolver = new Mono.Cecil.DefaultAssemblyResolver();
		assemblyResolver.AddSearchDirectory(System.IO.Path.GetDirectoryName(args[0]));
		Mono.Cecil.ReaderParameters readerParameters = new Mono.Cecil.ReaderParameters { AssemblyResolver = assemblyResolver };
		Mono.Cecil.AssemblyDefinition assembly = Mono.Cecil.AssemblyDefinition.ReadAssembly(args[0], readerParameters);
		foreach(Mono.Cecil.TypeDefinition type in assembly.MainModule.GetTypes()) {
			System.Collections.Generic.HashSet<string> names = System.Linq.Enumerable.ToHashSet(typeLookup[type.Name]);
			if(names.Contains("$"))
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
		assembly.MainModule.Write(args[1]);
	}
}
