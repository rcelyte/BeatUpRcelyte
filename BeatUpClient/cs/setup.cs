static partial class BeatUpClient {
	const ulong MpCore_major = 1, MpCore_minor = 1, MpCore_patch = 0;

	[System.AttributeUsage(System.AttributeTargets.Method)]
	internal class InitAttribute : System.Attribute {}

	internal static uint GatherMethods(System.Type type, ref System.Action patches, ref System.Action inits) {
		// Log.Debug($"GatherMethods({type})");
		uint patchCount = 0;
		foreach(System.Type nested in type.GetNestedTypes(HarmonyLib.AccessTools.all))
			patchCount += GatherMethods(nested, ref patches, ref inits);
		if(type.IsInterface)
			return patchCount;
		foreach(System.Reflection.MethodInfo method in type.GetMethods(System.Reflection.BindingFlags.DeclaredOnly | System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.Static)) {
			if(method.IsAbstract)
				continue;
			if(!method.ContainsGenericParameters)
				method.MethodHandle.GetFunctionPointer(); // Force JIT compilation
			foreach(System.Attribute attrib in method.GetCustomAttributes(false)) {
				if(attrib is Patch patch) {
					patches += () => patch.Apply(method);
					++patchCount;
				} else if(attrib is InitAttribute) {
					inits += (System.Action)System.Delegate.CreateDelegate(typeof(System.Action), method);
				}
			}
		}
		return patchCount;
	}
	static void OnSceneLoaded(UnityEngine.SceneManagement.Scene scene, UnityEngine.SceneManagement.LoadSceneMode mode) {
		if(scene.name != "MainMenu")
			return;
		Log.Debug("load MainMenu");
		if(Resolve<CustomNetworkConfig>() != null)
			SelectorSetup();
		LobbyUISetup();
	}
	internal static void Enable(Hive.Versioning.Version version, System.Func<string, Hive.Versioning.Version?> modVersion) {
		uint protocolVersion = (uint)HarmonyLib.AccessTools.Field(typeof(NetworkConstants), nameof(NetworkConstants.kProtocolVersion)).GetValue(null);
		if(protocolVersion != 8u) {
			BeatUpClient_Error.Init("Incompatible BeatUpClient Version", $"This version of BeatUpClient requires a{((protocolVersion < 8u) ? " newer" : "n older")} version of Beat Saber.");
			return;
		}
		Hive.Versioning.Version? mpCoreVersion = modVersion("MultiplayerCore");
		if(mpCoreVersion != null && (mpCoreVersion.Major != MpCore_major || mpCoreVersion.Minor != MpCore_minor || mpCoreVersion.Patch != MpCore_patch)) {
			BeatUpClient_Error.Init("Incompatible BeatUpClient Version", $"This version of BeatUpClient only supports MultiplayerCore {MpCore_major}.{MpCore_minor}.{MpCore_patch}");
			return;
		}
		string? err = BeatUpClient_Beta.CheckVersion(version);
		if(err != null) {
			if(err.Length != 0)
				BeatUpClient_Error.Init("Unsupported BeatUpClient Version", err);
			else
				BeatUpClient_Error.Init("BeatUpClient Validation Error", "BeatUpClient encountered a critical error. Please message rcelyte#5372.");
			return;
		}

		string localization = "Polyglot\t100\n" +
			"BEATUP_COUNTDOWN_DURATION\t\tCountdown Duration\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_SKIP_RESULTS_PYRAMID\t\tSkip Results Pyramid\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_PER_PLAYER_DIFFICULTY\t\tPer-Player Difficulty\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_PER_PLAYER_MODIFIERS\t\tPer-Player Modifiers\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_ADD_SERVER\t\tAdd Server\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_EDIT_SERVER\t\tEdit Server\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_ENTER_HOSTNAME\t\tEnter Hostname\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_DIRECT_DOWNLOADS\t\t<color=red>[Experimental]</color> Direct Downloads\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_ENABLE_DIRECT_DOWNLOADS\t\tEnable downloading unmodded custom levels directly from other BeatUpClient users.<br>WARNING: songs may fail to load, and may break BeatTogether lobbies.\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_HIDE_OTHER_LEVELS\t\tHide notes from other players\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_MAY_IMPROVE_PERFORMANCE\t\tMay improve performance\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			$"BEATUP_INFO\t\tBeatUpClient {version} <color=red>| BETA</color> is active.<br>If any issues arise, please contact rcelyte#5372 <b>immediately</b>.\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_SELECTED_MODIFIERS\t\tSelected Modifiers\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_VOTE_MODIFIERS\t\tSuggested Modifiers (Vote)\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_SWITCH\t\tSwitch\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_LARGE_LOBBY_AUTOKICK\t\tMultiplayerCore is required to play in lobbies with more than 5 players\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n";
		Polyglot.LocalizationImporter.Import(localization, Polyglot.GoogleDriveDownloadFormat.TSV);

		haveSiraUtil = modVersion("SiraUtil") != null;
		haveSongCore = modVersion("SongCore") != null;
		haveMpCore = modVersion("MultiplayerCore") != null;
		haveMpEx = modVersion("MultiplayerExtensions") != null;
		Log.Debug($"haveSiraUtil={haveSiraUtil}");
		Log.Debug($"haveSongCore={haveSongCore}");
		Log.Debug($"haveMpCore={haveMpCore}");
		Log.Debug($"haveMpEx={haveMpEx}");

		try {
			System.Collections.Generic.List<System.Type> sections = new System.Collections.Generic.List<System.Type>();
			sections.Add(typeof(BeatUpClient));
			if(haveMpCore)
				sections.Add(typeof(BeatUpClient_MpCore));
			Log.Debug("Gathering patches");
			uint patchCount = 0;
			System.Action applyPatches = delegate {}, init = delegate {};
			foreach(System.Type type in sections)
				patchCount += GatherMethods(type, ref applyPatches, ref init);
			Log.Debug("Loading assets");
			UnityEngine.AssetBundle data = UnityEngine.AssetBundle.LoadFromStream(System.Reflection.Assembly.GetExecutingAssembly().GetManifestResourceStream("BeatUpClient.data"));
			UnityEngine.Sprite[] sprites = data.LoadAllAssets<UnityEngine.Sprite>();
			defaultPackCover = sprites[0];
			altCreateButtonSprites[0] = sprites[1];
			altCreateButtonSprites[1] = sprites[2];
			altCreateButtonSprites[2] = sprites[2];
			altCreateButtonSprites[3] = sprites[1];
			Log.Debug($"Applying {patchCount} patches");
			init();
			if(haveMpCore)
				BeatUpClient_MpCore.Patch();
			applyPatches();
			UnityEngine.SceneManagement.SceneManager.sceneLoaded += OnSceneLoaded;
		} catch(System.Exception ex) {
			Log.Error($"Error applying patches: {ex}");
			Disable();
		}
	}
	internal static void Disable() {
		try {
			UnityEngine.SceneManagement.SceneManager.sceneLoaded -= OnSceneLoaded;
			harmony.UnpatchSelf();
			if(haveMpCore)
				BeatUpClient_MpCore.Unpatch();
		} catch(System.Exception ex) {
			Log.Error($"Error removing patches: {ex}");
		}
	}
}
