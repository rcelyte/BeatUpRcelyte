#if MPCORE_SUPPORT
using static BeatUpClient;

static partial class BeatUpClient_MpCore {
	[Patch(PatchType.Prefix, typeof(MultiplayerCore.Objects.MpEntitlementChecker), nameof(MultiplayerCore.Objects.MpEntitlementChecker.HandleGetIsEntitledToLevel))]
	public static bool MpEntitlementChecker_HandleGetIsEntitledToLevel(MultiplayerCore.Objects.MpEntitlementChecker __instance, string levelId, IMenuRpcManager ____rpcManager) {
		Log.Debug($"MpEntitlementChecker_HandleGetIsEntitledToLevel(levelId=\"{levelId}\")");
		HandleGetIsEntitledToLevel(__instance, levelId, ____rpcManager);
		return false;
	}

	[Patch.Overload(PatchType.Prefix, typeof(Newtonsoft.Json.JsonConvert), nameof(Newtonsoft.Json.JsonConvert.DeserializeObject), new[] {typeof(string), typeof(System.Type), typeof(Newtonsoft.Json.JsonSerializerSettings)})]
	public static void JsonConvert_DeserializeObject(ref System.Type type) { // TODO: remove if https://github.com/Goobwabber/MultiplayerCore/pull/19 gets merged
		if(type == typeof(MultiplayerStatusData))
			type = typeof(MultiplayerCore.Models.MpStatusData);
	}

	/*public static async System.Threading.Tasks.Task<EntitlementsStatus> MpShareWrapper(System.Threading.Tasks.Task<EntitlementsStatus> status, string levelId, NetworkPlayerEntitlementChecker checker) {
		switch(await status) {
			case EntitlementsStatus.Ok: return await ShareTask(levelId);
			case EntitlementsStatus.NotOwned: return await checker._additionalContentModel.GetLevelEntitlementStatusAsync(levelId, System.Threading.CancellationToken.None) switch {
					AdditionalContentModel.EntitlementStatus.Owned => await ShareTask(levelId),
					AdditionalContentModel.EntitlementStatus.NotOwned => EntitlementsStatus.NotOwned,
					_ => EntitlementsStatus.Unknown,
			};
			default: return status.Result;
		};
	}

	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Objects.MpEntitlementChecker), nameof(MultiplayerCore.Objects.MpEntitlementChecker.GetEntitlementStatus))]
	public static void MpEntitlementChecker_GetEntitlementStatus(MultiplayerCore.Objects.MpEntitlementChecker __instance, ref System.Threading.Tasks.Task<EntitlementsStatus> __result, string levelId) {
		Log.Debug($"MpEntitlementChecker_GetEntitlementStatus(levelId=\"{levelId}\")");
		__result = MpShareWrapper(__result, levelId, __instance);
	}*/

	static System.Diagnostics.Stopwatch rateLimit = System.Diagnostics.Stopwatch.StartNew();
	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Objects.MpLevelLoader), nameof(MultiplayerCore.Objects.MpLevelLoader.Report))]
	public static void MpLevelLoader_Report(double value) {
		if(rateLimit.ElapsedMilliseconds < 28)
			return;
		rateLimit.Restart();
		Net.SetLocalProgressUnreliable(new LoadProgress(LoadState.Downloading, (ushort)(value * 65535)));
	}

	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Objects.MpLevelLoader), nameof(MultiplayerCore.Objects.MpLevelLoader.LoadLevel))]
	public static void MpLevelLoader_LoadLevel_post(ILevelGameplaySetupData gameplaySetupData, System.Threading.CancellationTokenSource ____getBeatmapCancellationTokenSource, ref System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> ____getBeatmapLevelResultTask) =>
		MultiplayerLevelLoader_LoadLevel_post(true, gameplaySetupData, ____getBeatmapCancellationTokenSource, ref ____getBeatmapLevelResultTask);

	[Patch.Overload(PatchType.Postfix, typeof(MultiplayerCore.Patchers.NetworkConfigPatcher), nameof(MultiplayerCore.Patchers.NetworkConfigPatcher.UseMasterServer), new[] {typeof(DnsEndPoint), typeof(string), typeof(System.Nullable<int>)})]
	[Patch.Overload(PatchType.Postfix, typeof(MultiplayerCore.Patchers.NetworkConfigPatcher), nameof(MultiplayerCore.Patchers.NetworkConfigPatcher.UseMasterServer), new[] {typeof(DnsEndPoint), typeof(string), typeof(System.Nullable<int>), typeof(string)})]
	public static void NetworkConfigPatcher_UseMasterServer(DnsEndPoint endPoint, string statusUrl) =>
		UpdateNetworkConfig(endPoint.ToString(), statusUrl);

	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Patchers.NetworkConfigPatcher), nameof(MultiplayerCore.Patchers.NetworkConfigPatcher.UseOfficialServer))]
	public static void NetworkConfigPatcher_UseOfficialServer() =>
		UpdateNetworkConfig(string.Empty, null);

	public static void Patch() {
		MultiplayerCore.Patches.DataModelBinderPatch._playersDataModelMethod = HarmonyLib.AccessTools.DeclaredMethod(typeof(BeatUpClient_MpCore), nameof(BeatUpClient_MpCore.Patch)); // Suppress transpiler
		DiJack.Register(typeof(PlayersDataModel));
		DiJack.Patch();
	}
	public static void Unpatch() =>
		DiJack.UnregisterAll();
}
#endif
