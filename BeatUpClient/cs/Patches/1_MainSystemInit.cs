static partial class BeatUpClient {
	[Detour(typeof(MainSystemInit), nameof(MainSystemInit.InstallBindings))]
	static void MainSystemInit_InstallBindings(MainSystemInit self, Zenject.DiContainer container, bool isRunningFromTests) {
		Base(self, container, isRunningFromTests);

		BeatSaberMultiplayerSessionManager? multiplayerSessionManager = Injected<BeatSaberMultiplayerSessionManager>.Resolve(container);
		IMenuRpcManager menuRpcManager = Injected<IMenuRpcManager>.Resolve(container)!;
		Injected<AudioClipAsyncLoader>.Resolve(container);
		Injected<BeatmapDataLoader>.Resolve(container);
		multiplayerSessionManager?.SetLocalPlayerState("modded", true);
		Net.Setup(menuRpcManager, multiplayerSessionManager);
	}

	[Detour(typeof(MainSettingsAsyncLoader), nameof(MainSettingsAsyncLoader.InstallBindings))]
	static void MainSettingsAsyncLoader_InstallBindings(MainSettingsAsyncLoader self) {
		Base(self);

		Zenject.DiContainer container = self.Container;
		SettingsManager settingsManager = Injected<SettingsManager>.Resolve(container)!;
		string hostname = settingsManager.settings.customServer.hostName?.ToLower() ?? string.Empty;
		int port = self._networkConfig.masterServerPort;
		if(hostname.Contains(":") == true) {
			int.TryParse(hostname.Split(':')[1], out port);
			hostname = hostname.Split(':')[0];
		}
		container.Rebind<INetworkConfig>().FromInstance(new CustomNetworkConfig(self._networkConfig, hostname, port, true)).AsSingle();
		NetworkConfigSetup(self._networkConfig);
		Injected<CustomNetworkConfig>.Resolve<INetworkConfig>(container);
		Injected<IMultiplayerStatusModel>.Resolve(container);
		Injected<IQuickPlaySetupModel>.Resolve(container);
		Injected<BGNet.Core.GameLift.GameLiftPlayerSessionProvider>.Resolve<BGNet.Core.GameLift.IGameLiftPlayerSessionProvider>(container);
	}

	[Detour(typeof(CustomLevelsSettingsAsyncInstaller), nameof(CustomLevelsSettingsAsyncInstaller.InstallBindings))]
	static void CustomLevelsSettingsAsyncInstaller_InstallBindings(CustomLevelsSettingsAsyncInstaller self) {
		Base(self);
		Injected<BeatmapCharacteristicCollection>.Resolve(self.Container);
		Injected<CustomLevelLoader>.Resolve(self.Container);
		Injected<BeatmapLevelsModel>.Resolve(self.Container);
	}
}
