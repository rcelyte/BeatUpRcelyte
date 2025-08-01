static partial class BeatUpClient {
	[Detour(typeof(BGLib.Polyglot.LocalizationImporter), nameof(BGLib.Polyglot.LocalizationImporter.ImportFromFiles))]
	static System.Collections.Generic.Dictionary<string, System.Collections.Generic.List<string>> LocalizationImporter_ImportFromFiles(System.Collections.Generic.List<BGLib.Polyglot.LocalizationAsset> inputFiles) {
		inputFiles.Add(new(new UnityEngine.TextAsset("Polyglot,100\n" +
			"BEATUP_COUNTDOWN_DURATION,,Countdown Duration,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_SKIP_RESULTS_PYRAMID,,Skip Results Pyramid,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_PER_PLAYER_DIFFICULTY,,Per-Player Difficulty,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_PER_PLAYER_MODIFIERS,,Per-Player Modifiers,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_ADD_SERVER,,Add Server,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_EDIT_SERVER,,Edit Server,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_ENTER_HOSTNAME,,Enter Hostname,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_DIRECT_DOWNLOADS,,<color=red>[Experimental]</color> Direct Downloads,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_ENABLE_DIRECT_DOWNLOADS,,Enable downloading unmodded custom levels directly from other BeatUpClient users.<br>WARNING: songs may fail to load\",\" and may break BeatTogether lobbies.,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_HIDE_OTHER_LEVELS,,Hide notes from other players,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_MAY_IMPROVE_PERFORMANCE,,May improve performance,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			$"BEATUP_INFO,,BeatUpClient {modVersion} <color=red>| BETA</color> is active.<br>If any issues arise\",\" please contact <color=lightblue>@rcelyte</color> on Discord.,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_SELECTED_MODIFIERS,,Selected Modifiers,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_SWITCH,,Switch,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n" +
			"BEATUP_LARGE_LOBBY_AUTOKICK,,MultiplayerCore is required to play in lobbies with more than 5 players,"+/*French*/","+/*Spanish*/","+/*German*/",,,,,,,,,,,,,"+/*Japanese*/","+/*Simplified Chinese*/",,"+/*Korean*/",,,,,,,,\n")));
		return (System.Collections.Generic.Dictionary<string, System.Collections.Generic.List<string>>)Base(inputFiles);
	}
}
