static partial class BeatUpClient {
	internal struct VanillaConfig : INetworkConfig { // `NetworkConfigSO` getters are hooked by MultiplayerCore, so avoid using that type
		public int maxPartySize {get; set;}
		public int discoveryPort {get;}
		public int partyPort {get;}
		public int multiplayerPort {get;}
		public int masterServerPort {get; set;}
		public string multiplayerStatusUrl {get; set;}
		public string quickPlaySetupUrl {get;}
		public string graphUrl {get; set;}
		public string graphQLUrl => graphUrl + "/graphql";
		public string graphAccessToken {get;}
		public ulong graphAppId {get;}
		public bool forceGameLift {get; set;}
		public int localServerPort {get;}
		public bool useLocalServer {get;}
		public ServiceEnvironment serviceEnvironment {get;}
		public string customLocation {get;}
		public VanillaConfig(NetworkConfigSO from) => // read private fields directly
			(this.maxPartySize, this.discoveryPort, this.partyPort, this.multiplayerPort, this.masterServerPort, this.multiplayerStatusUrl, this.quickPlaySetupUrl, this.graphUrl,
					this.graphAccessToken, this.graphAppId, this.forceGameLift, this.localServerPort, this.useLocalServer, this.serviceEnvironment, this.customLocation) = 
				(5, from._discoveryPort, from._partyPort, from._multiplayerPort, from._masterServerPort, from._multiplayerStatusUrl, from._quickPlaySetupUrl, from._graphUrl,
					from.graphAccessToken, from._graphAppId, from._forceGameLift, from._localServerPort, from._useLocalServer, from._serviceEnvironment, from._customLocation);
	}

	internal static VanillaConfig? officialConfig = null;
	static void NetworkConfigSetup(NetworkConfigSO networkConfig) {
		if(officialConfig == null)
			officialConfig = new VanillaConfig(networkConfig);
	}

	static bool SetNetworkConfig(string hostname, string statusUrl) {
		Resolve<SettingsManager>()!.settings.customServer.hostName = hostname;
		CustomNetworkConfig? customNetworkConfig = Resolve<CustomNetworkConfig>();
		if(customNetworkConfig == null)
			return false;
		VanillaConfig newConfig = officialConfig ?? default;
		if(!string.IsNullOrEmpty(hostname)) {
			string[] splitHostname = hostname.ToLower().Split(new[] {':'});
			int port = newConfig.masterServerPort;
			if(splitHostname.Length >= 2)
				if(int.TryParse(splitHostname[1], out int customPort) && customPort > 0)
					port = customPort;
			newConfig.maxPartySize = 254;
			newConfig.masterServerPort = port;
			newConfig.multiplayerStatusUrl = string.IsNullOrEmpty(statusUrl) ? "https://status." + splitHostname[0] : statusUrl;
			newConfig.graphUrl = (hostname.StartsWith("http://") || hostname.StartsWith("https://")) ? hostname : newConfig.multiplayerStatusUrl;
			newConfig.forceGameLift = true;
		}
		string oldGraphUrl = customNetworkConfig.graphUrl, oldStatusUrl = customNetworkConfig.multiplayerStatusUrl;
		typeof(CustomNetworkConfig).GetConstructors()[0].Invoke(customNetworkConfig, new object[] {newConfig, "", 0, true});
		HarmonyLib.AccessTools.Field(typeof(CustomNetworkConfig), "<graphAppId>k__BackingField").SetValue(customNetworkConfig, (ulong)newConfig.graphAppId);
		HarmonyLib.AccessTools.Field(typeof(CustomNetworkConfig), "<forceGameLift>k__BackingField").SetValue(customNetworkConfig, (bool)newConfig.forceGameLift);
		if(Resolve<BGNet.Core.GameLift.GameLiftPlayerSessionProvider>() is BGNet.Core.GameLift.GameLiftPlayerSessionProvider provider &&
				provider._networkConfig == customNetworkConfig)
			provider._graphAPIClient = new GraphAPIClient(customNetworkConfig.graphUrl, customNetworkConfig.graphAccessToken);
		if(customNetworkConfig.multiplayerStatusUrl == oldStatusUrl)
			return newConfig.graphUrl != oldGraphUrl;
		if(Resolve<IMultiplayerStatusModel>() is MultiplayerStatusModel multiplayerStatusModel)
			multiplayerStatusModel._request = null;
		if(Resolve<IQuickPlaySetupModel>() is QuickPlaySetupModel quickPlaySetupModel)
			quickPlaySetupModel._request = null;
		return true;
	}

	static System.Action<string, string>? onNetworkConfigChanged = null;
	internal static bool UpdateNetworkConfig(string hostname, string statusUrl = "") {
		bool changed = SetNetworkConfig(hostname, statusUrl);
		onNetworkConfigChanged?.Invoke(hostname, statusUrl);
		return changed;
	}

	[Detour(typeof(IgnoranceCore.IgnoranceClient), nameof(IgnoranceCore.IgnoranceClient.Start))]
	static void IgnoranceClient_Start(IgnoranceCore.IgnoranceClient self) {
		if(!currentServerIsOfficial) {
			self.UseSsl = currentServerIsBeatUp; // TODO: read `useSsl` value from MultiplayerStatusData
			self.ValidateCertificate = false;
		}
		Base(self);
	}
}
