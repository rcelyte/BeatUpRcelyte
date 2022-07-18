static partial class BeatUpClient {
	struct VanillaNetworkConfig : INetworkConfig {
		public int maxPartySize {get;}
		public int discoveryPort {get;}
		public int partyPort {get;}
		public int multiplayerPort {get;}
		public DnsEndPoint masterServerEndPoint {get;}
		public string multiplayerStatusUrl {get;}
		public string quickPlaySetupUrl {get;}
		public string graphUrl {get;}
		public string graphAccessToken {get;}
		public bool forceGameLift {get;}
		public ServiceEnvironment serviceEnvironment {get;}
		public VanillaNetworkConfig(NetworkConfigSO from) => // Bypass all getters other mods might patch
			(this.maxPartySize, this.discoveryPort, this.partyPort, this.multiplayerPort, this.masterServerEndPoint, this.multiplayerStatusUrl, this.quickPlaySetupUrl, this.graphUrl, this.graphAccessToken, this.forceGameLift, this.serviceEnvironment) = 
				(from._maxPartySize, from._discoveryPort, from._partyPort, from._multiplayerPort, new DnsEndPoint(from._masterServerHostName, from._masterServerPort), from._multiplayerStatusUrl, from._quickPlaySetupUrl, from.graphUrl, from.graphAccessToken, from._forceGameLift, from._serviceEnvironment);
	}

	static VanillaNetworkConfig officialNetworkConfig;
	static void NetworkConfigSetup(NetworkConfigSO networkConfig) {
		if(officialNetworkConfig.masterServerEndPoint == null)
			officialNetworkConfig = new VanillaNetworkConfig(networkConfig);
	}

	static bool SetNetworkConfig(string? hostname, string? statusUrl) {
		customServerHostName.value = hostname ?? string.Empty;
		editServerButton.interactable = false;
		CustomNetworkConfig? customNetworkConfig = Resolve<CustomNetworkConfig>();
		if(customNetworkConfig == null)
			return false;
		int port = officialNetworkConfig.masterServerEndPoint.port;
		bool forceGameLift = officialNetworkConfig.forceGameLift;
		string? multiplayerStatusUrl = officialNetworkConfig.multiplayerStatusUrl;
		if(NullableStringHelper.IsNullOrEmpty(hostname)) {
			hostname = officialNetworkConfig.masterServerEndPoint.hostName;
		} else {
			editServerButton.interactable = true;
			string[] splitHostname = hostname.ToLower().Split(new[] {':'});
			hostname = splitHostname[0];
			if(splitHostname.Length >= 2)
				int.TryParse(splitHostname[1], out port);
			forceGameLift = false;
		}
		string oldHostname = customNetworkConfig.masterServerEndPoint.hostName;
		int oldPort = customNetworkConfig.masterServerEndPoint.port;
		string oldMultiplayerStatusUrl = customNetworkConfig.multiplayerStatusUrl;
		Log.Debug($"CustomNetworkConfig(customServerHostName=\"{hostname}\", port={port}, forceGameLift={forceGameLift}), multiplayerStatusUrl={statusUrl}");
		typeof(CustomNetworkConfig).GetConstructors()[0].Invoke(customNetworkConfig, new object[] {officialNetworkConfig, hostname, port, forceGameLift});
		if(!NullableStringHelper.IsNullOrEmpty(statusUrl))
			HarmonyLib.AccessTools.Field(typeof(CustomNetworkConfig), "<multiplayerStatusUrl>k__BackingField").SetValue(customNetworkConfig, (string)statusUrl);
		if(!forceGameLift)
			HarmonyLib.AccessTools.Field(typeof(CustomNetworkConfig), "<serviceEnvironment>k__BackingField").SetValue(customNetworkConfig, officialNetworkConfig.serviceEnvironment);
		if(customNetworkConfig.multiplayerStatusUrl == oldMultiplayerStatusUrl)
			return hostname != oldHostname || port != oldPort;

		if(Resolve<IMultiplayerStatusModel>() is MultiplayerStatusModel multiplayerStatusModel)
			multiplayerStatusModel._request = null;
		if(Resolve<IQuickPlaySetupModel>() is QuickPlaySetupModel quickPlaySetupModel)
			quickPlaySetupModel._request = null;
		return true;
	}

	internal static void UpdateNetworkConfig(string hostname, string? statusUrl) {
		SetNetworkConfig(hostname, statusUrl);
		serverDropdown?.SetServer(customServerHostName, statusUrl);
	}
}
