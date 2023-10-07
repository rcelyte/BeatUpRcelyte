static partial class BeatUpClient {
	[Patch(PatchType.Postfix, typeof(MainSystemInit), nameof(MainSystemInit.InstallBindings))]
	static void MainSystemInit_InstallBindings_post(MainSystemInit __instance, Zenject.DiContainer container) {
		customServerHostName = __instance._mainSettingsModel.customServerHostName;
		string hostname = customServerHostName.value.ToLower();
		int port = __instance._networkConfig.masterServerEndPoint.port;
		if(hostname.Contains(":")) {
			int.TryParse(hostname.Split(':')[1], out port);
			hostname = hostname.Split(':')[0];
		}
		container.Rebind<INetworkConfig>().FromInstance(new CustomNetworkConfig(__instance._networkConfig, hostname, port, true)).AsSingle();

		// TODO: everything below should run later (i.e. @ AudioClipAsyncLoader..ctor)
		NetworkConfigSetup(__instance._networkConfig);
		Injected<BeatmapCharacteristicCollection>.Resolve(container);
		Injected<BeatmapLevelsModel>.Resolve(container);
		Injected<CustomLevelLoader>.Resolve(container);
		Injected<CustomNetworkConfig>.Resolve<INetworkConfig>(container);
		Injected<IMultiplayerStatusModel>.Resolve(container);
		Injected<IQuickPlaySetupModel>.Resolve(container);
		MultiplayerSessionManager? multiplayerSessionManager = Injected<MultiplayerSessionManager>.Resolve<IMultiplayerSessionManager>(container);
		IMenuRpcManager menuRpcManager = Injected<IMenuRpcManager>.Resolve(container)!;
		multiplayerSessionManager?.SetLocalPlayerState("modded", true);
		Net.Setup(menuRpcManager, multiplayerSessionManager);
	}
}
