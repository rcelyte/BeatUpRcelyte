#if MPCORE_SUPPORT
using static BeatUpClient;
using static System.Linq.Enumerable;

static partial class BeatUpClient_MpCore {
	static System.Collections.Generic.HashSet<System.Type> Suppressions = new System.Collections.Generic.HashSet<System.Type> {
		typeof(MultiplayerCore.Objects.MpPlayersDataModel),
		typeof(MultiplayerCore.Patchers.CustomLevelsPatcher),
		typeof(MultiplayerCore.Patchers.ModeSelectionPatcher),
		typeof(MultiplayerCore.Patchers.NetworkConfigPatcher), // TODO: likely isn't needed since BeatUpClient uses CustomNetworkConfig instead of the patched NetworkConfigSO
		typeof(MultiplayerCore.Patches.OverridePatches.PlayersDataModelOverride),
	};

	[Patch(PatchType.Prefix, typeof(SiraUtil.Affinity.Harmony.HarmonyAffinityPatcher), nameof(SiraUtil.Affinity.Harmony.HarmonyAffinityPatcher.Patch))]
	public static bool HarmonyAffinityPatcher_Patch(SiraUtil.Affinity.IAffinity affinity) =>
		(!Suppressions.Contains(affinity.GetType())).Or(() => Log.Debug($"Suppressing {affinity.GetType()}"));

	static void PatchAll(HarmonyLib.Harmony self, System.Reflection.Assembly assembly) {
		foreach(System.Type type in HarmonyLib.AccessTools.GetTypesFromAssembly(assembly)) {
			if(Suppressions.Contains(type))
				Log.Debug($"Suppressing Harmony patches for {type}");
			else
				self.CreateClassProcessor(type).Patch();
		}
	}

	[Patch(PatchType.Transpiler, typeof(MultiplayerCore.Plugin), nameof(MultiplayerCore.Plugin.OnEnable))]
	public static System.Collections.Generic.IEnumerable<HarmonyLib.CodeInstruction> Plugin_OnEnable(System.Collections.Generic.IEnumerable<HarmonyLib.CodeInstruction> instructions) {
		System.Reflection.MethodInfo original = HarmonyLib.AccessTools.Method(typeof(HarmonyLib.Harmony), nameof(HarmonyLib.Harmony.PatchAll), new[] {typeof(System.Reflection.Assembly)});
		System.Reflection.MethodInfo replace = HarmonyLib.AccessTools.Method(typeof(BeatUpClient_MpCore), nameof(BeatUpClient_MpCore.PatchAll));
		foreach(HarmonyLib.CodeInstruction instruction in instructions) {
			if(HarmonyLib.CodeInstructionExtensions.Calls(instruction, original))
				yield return new HarmonyLib.CodeInstruction(System.Reflection.Emit.OpCodes.Call, replace);
			else
				yield return instruction;
		}
	}
}
#endif
