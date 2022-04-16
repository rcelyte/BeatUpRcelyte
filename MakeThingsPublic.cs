class MainClass {
	static string[] overrides = new[] {
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
		"IncDecSettingsController:_stepValuePicker",
		"InputFieldView:_placeholderText",
		"InputFieldView:_textLengthLimit",
		"JoiningLobbyViewController:_text",
		"LocalizationImporter:Import",
		"MainSystemInit:_networkConfig",
		"MpLevelLoader:.ctor",
		"MpPlayersDataModel:.ctor",
		"MultiplayerModeSelectionViewController:_customServerEndPointText",
		"MultiplayerModeSelectionViewController:_maintenanceMessageText",
		"NetConnectAcceptPacket",
		"NetPacket",
		"PlayerConnectedPacket",
		"StandardLevelDetailView:_beatmapCharacteristicSegmentedControlController",
		"StandardLevelDetailView:_beatmapDifficultySegmentedControlController",
		"SwitchSettingsController:_toggle",
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
			foreach(Mono.Cecil.FieldDefinition field in type.Fields)
				if(names.Contains(field.Name))
					field.IsPublic = true;
		}
		assembly.MainModule.Write(args[1]);
	}
}
