#if MPCORE_SUPPORT
using static BeatUpClient;

static partial class BeatUpClient_MpCore {
	[Patch(PatchType.Prefix, typeof(MultiplayerCore.Objects.MpEntitlementChecker), nameof(MultiplayerCore.Objects.MpEntitlementChecker.HandleGetIsEntitledToLevel))]
	public static bool MpEntitlementChecker_HandleGetIsEntitledToLevel(MultiplayerCore.Objects.MpEntitlementChecker __instance, string levelId, IMenuRpcManager ____rpcManager) {
		Log.Debug($"MpEntitlementChecker_HandleGetIsEntitledToLevel(levelId=\"{levelId}\")");
		// TODO: revert once we have an updated MultiplayerCore release to build against
		HandleGetIsEntitledToLevel(/*__instance.GetEntitlementStatus(levelId)*/(System.Threading.Tasks.Task<EntitlementsStatus>)typeof(MultiplayerCore.Objects.MpEntitlementChecker).GetMethod("GetEntitlementStatus").Invoke(__instance, new object[] {levelId}), levelId, ____rpcManager);
		return false;
	}

	static System.Diagnostics.Stopwatch rateLimit = System.Diagnostics.Stopwatch.StartNew();
	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Objects.MpLevelLoader), nameof(MultiplayerCore.Objects.MpLevelLoader.Report))]
	static void MpLevelLoader_Report(double value) {
		if(rateLimit.ElapsedMilliseconds < 28)
			return;
		rateLimit.Restart();
		Net.SetLocalProgressUnreliable(new LoadProgress(LoadState.Downloading, (ushort)(value * 65535)));
	}

	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Patchers.NetworkConfigPatcher), nameof(MultiplayerCore.Patchers.NetworkConfigPatcher.UseCustomApiServer))]
	static void NetworkConfigPatcher_UseCustomApiServer(string graphUrl, string statusUrl/*, int? maxPartySize, string? quickPlaySetupUrl, bool disableSsl*/) =>
		UpdateNetworkConfig(graphUrl, statusUrl); // TODO: respect `maxPartySize`, `quickPlaySetupUrl`

	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Patchers.NetworkConfigPatcher), nameof(MultiplayerCore.Patchers.NetworkConfigPatcher.UseOfficialServer))]
	public static void NetworkConfigPatcher_UseOfficialServer() =>
		UpdateNetworkConfig(string.Empty);

	static System.Exception? MultiplayerStatusModel_MoveNext() => null;

	[Detour(typeof(MultiplayerCore.Objects.MpPlayersDataModel), nameof(MultiplayerCore.Objects.MpPlayersDataModel.Activate))]
	static void MpPlayersDataModel_Activate(MultiplayerCore.Objects.MpPlayersDataModel self) {
		Log.Debug($"MpPlayersDataModel.Activate()");
		((LobbyPlayersDataModel)self).Activate();
	}

	[Detour(typeof(MultiplayerCore.Objects.MpPlayersDataModel), nameof(MultiplayerCore.Objects.MpPlayersDataModel.Deactivate))]
	static void MpPlayersDataModel_Deactivate(MultiplayerCore.Objects.MpPlayersDataModel self) {
		Log.Debug($"MpPlayersDataModel.Deactivate()");
		((LobbyPlayersDataModel)self).Deactivate();
	}

	[Detour(typeof(MultiplayerCore.Patchers.NetworkConfigPatcher), "get_IsOverridingApi")]
	static bool NetworkConfigPatcher_get_IsOverridingApi(MultiplayerCore.Patchers.NetworkConfigPatcher self) {
		CustomNetworkConfig? customNetworkConfig = Resolve<CustomNetworkConfig>();
		if(customNetworkConfig == null)
			return false;
		return customNetworkConfig.graphUrl != officialConfig?.graphUrl;
	}

	[Detour(typeof(MultiplayerCore.UI.MpPerPlayerUI), nameof(MultiplayerCore.UI.MpPerPlayerUI.Initialize))]
	static void MpPerPlayerUI_Initialize(MultiplayerCore.UI.MpPerPlayerUI self) {
		Base(self);
		self.segmentVert?.transform.SetParent(null); // disable MultiplayerCore's selector since it's missing characteristic buttons
	}

	[Init]
	public static void Patch() {
		new Patch(PatchType.Finalizer, typeof(MultiplayerStatusModel).Assembly.GetType("MultiplayerStatusModel+<GetMultiplayerStatusAsyncInternal>d__9", true), "MoveNext")
			.Bind(HarmonyLib.AccessTools.DeclaredMethod(typeof(BeatUpClient_MpCore), nameof(MultiplayerStatusModel_MoveNext)))();
	}
}
#endif
