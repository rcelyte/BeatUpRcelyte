static partial class BeatUpClient {
	static readonly BoolSO customServerEnvironmentOverride = UnityEngine.ScriptableObject.CreateInstance<BoolSO>();
	[Patch(PatchType.Transpiler, typeof(MainSystemInit), nameof(MainSystemInit.InstallBindings))]
	public static System.Collections.Generic.IEnumerable<HarmonyLib.CodeInstruction> MainSystemInit_InstallBindings(System.Collections.Generic.IEnumerable<HarmonyLib.CodeInstruction> instructions) {
		customServerEnvironmentOverride.value = true;
		System.Reflection.FieldInfo original = HarmonyLib.AccessTools.Field(typeof(MainSettingsModelSO), nameof(MainSettingsModelSO.useCustomServerEnvironment));
		System.Reflection.FieldInfo replace = HarmonyLib.AccessTools.Field(typeof(BeatUpClient), nameof(BeatUpClient.customServerEnvironmentOverride));
		foreach(HarmonyLib.CodeInstruction instruction in instructions) {
			if(HarmonyLib.CodeInstructionExtensions.LoadsField(instruction, original)) {
				yield return new HarmonyLib.CodeInstruction(System.Reflection.Emit.OpCodes.Pop);
				yield return new HarmonyLib.CodeInstruction(System.Reflection.Emit.OpCodes.Ldsfld, replace);
			} else {
				yield return instruction;
			}
		}
	}

	[Patch(PatchType.Postfix, typeof(MainSystemInit), nameof(MainSystemInit.InstallBindings))]
	static void MainSystemInit_InstallBindings_post(MainSystemInit __instance, Zenject.DiContainer container) {
		customServerHostName = __instance._mainSettingsModel.customServerHostName;
		NetworkConfigSetup(__instance._networkConfig);
		Injected<BeatmapCharacteristicCollectionSO>.Resolve(container);
		Injected<BeatmapLevelsModel>.Resolve(container);
		Injected<CustomLevelLoader>.Resolve(container);
		Injected<CustomNetworkConfig>.Resolve<INetworkConfig>(container);
		Injected<IMultiplayerStatusModel>.Resolve(container);
		Injected<IQuickPlaySetupModel>.Resolve(container);
		Injected<MultiplayerSessionManager>.Resolve<IMultiplayerSessionManager>(container)?.SetLocalPlayerState("modded", true);
		Net.Setup(Injected<IMenuRpcManager>.Resolve(container)!);
	}
}
