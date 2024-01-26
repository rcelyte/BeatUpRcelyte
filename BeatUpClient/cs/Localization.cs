static partial class BeatUpClient {
	[Detour(typeof(BGLib.Polyglot.LocalizationImporter), nameof(BGLib.Polyglot.LocalizationImporter.Initialize))]
	static void LocalizationImporter_Initialize(BGLib.Polyglot.LocalizationModel settings) {
		Base(settings);

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
			$"BEATUP_INFO\t\tBeatUpClient {modVersion} <color=red>| BETA</color> is active.<br>If any issues arise, please contact <color=lightblue>@rcelyte</color> on Discord.\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_SELECTED_MODIFIERS\t\tSelected Modifiers\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_SWITCH\t\tSwitch\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_LARGE_LOBBY_AUTOKICK\t\tMultiplayerCore is required to play in lobbies with more than 5 players\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n";
		BGLib.Polyglot.LocalizationImporter.Import(localization, BGLib.Polyglot.GoogleDriveDownloadFormat.TSV);
	}
}
