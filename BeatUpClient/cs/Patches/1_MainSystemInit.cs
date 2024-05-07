static partial class BeatUpClient {
	[Detour(typeof(MainSystemInit), nameof(MainSystemInit.InstallBindings))]
	static void MainSystemInit_InstallBindings(MainSystemInit self, Zenject.DiContainer container, bool isRunningFromTests) {
		Base(self, container, isRunningFromTests);

		MultiplayerSessionManager? multiplayerSessionManager = Injected<MultiplayerSessionManager>.Resolve<IMultiplayerSessionManager>(container);
		IMenuRpcManager menuRpcManager = Injected<IMenuRpcManager>.Resolve(container)!;
		Injected<AudioClipAsyncLoader>.Resolve(container);
		Injected<BeatmapDataLoader>.Resolve(container);
		multiplayerSessionManager?.SetLocalPlayerState("modded", true);
		Net.Setup(menuRpcManager, multiplayerSessionManager);
	}

	[Detour(typeof(MainSettingsAsyncLoader), nameof(MainSettingsAsyncLoader.InstallBindings))]
	static void MainSettingsAsyncLoader_InstallBindings(MainSettingsAsyncLoader self) {
		Base(self);

		customServerSettings = self._mainSettingsHandler.instance.customServerSettings;
		string hostname = customServerSettings.customServerHostName.ToLower();
		int port = self._networkConfig.masterServerEndPoint.port;
		if(hostname.Contains(":")) {
			int.TryParse(hostname.Split(':')[1], out port);
			hostname = hostname.Split(':')[0];
		}
		Zenject.DiContainer container = self.Container;
		container.Rebind<INetworkConfig>().FromInstance(new CustomNetworkConfig(self._networkConfig, hostname, port, true)).AsSingle();
		NetworkConfigSetup(self._networkConfig);
		Injected<CustomNetworkConfig>.Resolve<INetworkConfig>(container);
		Injected<IMultiplayerStatusModel>.Resolve(container);
		Injected<IQuickPlaySetupModel>.Resolve(container);
	}

	[Detour(typeof(BeatmapCharacteristicInstaller), nameof(BeatmapCharacteristicInstaller.InstallBindings))]
	static void BeatmapCharacteristicInstaller_InstallBindings(BeatmapCharacteristicInstaller self) {
		Base(self);
		Injected<BeatmapCharacteristicCollection>.Resolve(self.Container);
		Injected<CustomLevelLoader>.Resolve(self.Container);
		Injected<BeatmapLevelsModel>.Resolve(self.Container);
	}
}
