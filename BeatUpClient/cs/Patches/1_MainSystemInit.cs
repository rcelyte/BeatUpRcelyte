static partial class BeatUpClient {
	[Detour(typeof(BeatSaberInit), nameof(BeatSaberInit.InstallBindings))]
	static void BeatSaberInit_InstallBindings(BeatSaberInit self) {
		Base(self);
		Zenject.SceneContext.ExtraPostInstallMethod += container => {
			Injected<GameScenesManager>.Resolve(Zenject.ProjectContext.Instance.Container);
			BeatSaberMultiplayerSessionManager? multiplayerSessionManager = Injected<BeatSaberMultiplayerSessionManager>.Resolve(container);
			IMenuRpcManager menuRpcManager = Injected<IMenuRpcManager>.Resolve(container)!;
			Injected<AudioClipAsyncLoader>.Resolve(container);
			Injected<BeatmapDataLoader>.Resolve(container);
			Injected<BeatmapLevelsModel>.Resolve(container);
			Injected<BGNet.Core.GameLift.GameLiftPlayerSessionProvider>.Resolve<BGNet.Core.GameLift.IGameLiftPlayerSessionProvider>(container);
			Injected<CustomNetworkConfig>.Resolve<INetworkConfig>(container);
			Injected<IMultiplayerStatusModel>.Resolve(container);
			Injected<IQuickPlaySetupModel>.Resolve(container);
			Injected<MultiplayerLevelScenesTransitionSetupData>.Resolve(container);
			multiplayerSessionManager?.SetLocalPlayerState("modded", true);
			Net.Setup(menuRpcManager, multiplayerSessionManager);
		};
	}

	static NetworkConfigSO networkConfigSO = null!;
	[Detour(typeof(MainSettingsAsyncLoader), nameof(MainSettingsAsyncLoader.RegisterInstallers))]
	static void MainSettingsAsyncLoader_RegisterInstallers(MainSettingsAsyncLoader self, BGLib.AppFlow.Initialization.IInstallerRegistry registry) {
		Base(self, registry);
		networkConfigSO = self._networkConfig;
		NetworkConfigSetup(self._networkConfig);
	}

	[Detour(typeof(MainSettingsAsyncLoader.MainSettingsInstaller), nameof(MainSettingsAsyncLoader.MainSettingsInstaller.InstallBindings))]
	static void MainSettingsInstaller_InstallBindings(MainSettingsAsyncLoader.MainSettingsInstaller self) {
		Base(self);

		Zenject.DiContainer container = self.Container;
		SettingsManager settingsManager = Injected<SettingsManager>.Resolve(container)!;
		string hostname = settingsManager.settings.customServer.hostName?.ToLower() ?? string.Empty;
		int port = networkConfigSO.masterServerPort;
		if(hostname.Contains(":") == true) {
			int.TryParse(hostname.Split(':')[1], out port);
			hostname = hostname.Split(':')[0];
		}
		container.Rebind<INetworkConfig>().FromInstance(new CustomNetworkConfig(networkConfigSO, hostname, port, true)).AsSingle();
	}
}
