static partial class BeatUpClient {
	struct VanillaConfig : INetworkConfig {
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
		public VanillaConfig(NetworkConfigSO from) => // Bypass all getters other mods might patch
			(this.maxPartySize, this.discoveryPort, this.partyPort, this.multiplayerPort, this.masterServerEndPoint, this.multiplayerStatusUrl, this.quickPlaySetupUrl, this.graphUrl, this.graphAccessToken, this.forceGameLift, this.serviceEnvironment) = 
				(/*from._maxPartySize*/5, from._discoveryPort, from._partyPort, from._multiplayerPort, new DnsEndPoint(from._masterServerHostName, from._masterServerPort), from._multiplayerStatusUrl, from._quickPlaySetupUrl, from.graphUrl, from.graphAccessToken, from._forceGameLift, from._serviceEnvironment);
	}

	static VanillaConfig officialConfig;
	static void NetworkConfigSetup(NetworkConfigSO networkConfig) {
		if(officialConfig.masterServerEndPoint == null)
			officialConfig = new VanillaConfig(networkConfig);
	}

	static bool SetNetworkConfig(string hostname, string statusUrl) {
		customServerHostName.value = hostname;
		CustomNetworkConfig? customNetworkConfig = Resolve<CustomNetworkConfig>();
		if(customNetworkConfig == null)
			return false;
		int port = officialConfig.masterServerEndPoint.port;
		bool forceGameLift = true;
		string? multiplayerStatusUrl = officialConfig.multiplayerStatusUrl;
		bool custom = !string.IsNullOrEmpty(hostname);
		if(custom) {
			string[] splitHostname = hostname.ToLower().Split(new[] {':'});
			hostname = splitHostname[0];
			if(splitHostname.Length >= 2)
				if(int.TryParse(splitHostname[1], out int customPort) && customPort > 0)
					port = customPort;
			forceGameLift = false;
		} else {
			hostname = officialConfig.masterServerEndPoint.hostName;
		}
		string oldHostname = customNetworkConfig.masterServerEndPoint.hostName;
		int oldPort = customNetworkConfig.masterServerEndPoint.port;
		string oldMultiplayerStatusUrl = customNetworkConfig.multiplayerStatusUrl;
		Log.Debug($"CustomNetworkConfig(customServerHostName=\"{hostname}\", port={port}, forceGameLift={forceGameLift}), multiplayerStatusUrl={statusUrl}");
		typeof(CustomNetworkConfig).GetConstructors()[0].Invoke(customNetworkConfig, new object[] {officialConfig, hostname, port, forceGameLift});
		if(custom)
			HarmonyLib.AccessTools.Field(typeof(CustomNetworkConfig), "<maxPartySize>k__BackingField").SetValue(customNetworkConfig, (int)254);
		if(!string.IsNullOrEmpty(statusUrl))
			HarmonyLib.AccessTools.Field(typeof(CustomNetworkConfig), "<multiplayerStatusUrl>k__BackingField").SetValue(customNetworkConfig, (string)statusUrl);
		if(!forceGameLift)
			HarmonyLib.AccessTools.Field(typeof(CustomNetworkConfig), "<serviceEnvironment>k__BackingField").SetValue(customNetworkConfig, officialConfig.serviceEnvironment);
		if(customNetworkConfig.multiplayerStatusUrl == oldMultiplayerStatusUrl)
			return hostname != oldHostname || port != oldPort;

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

	[Detour(typeof(ClientCertificateValidator), nameof(ClientCertificateValidator.ValidateCertificateChainInternal))]
	static void ClientCertificateValidator_ValidateCertificateChainInternal(ClientCertificateValidator self, DnsEndPoint endPoint, System.Security.Cryptography.X509Certificates.X509Certificate2 certificate, byte[][] certificateChain) {
		if(Resolve<CustomNetworkConfig>() is CustomNetworkConfig networkConfig && networkConfig.masterServerEndPoint.hostName != officialConfig.masterServerEndPoint.hostName)
			return;
		Base(self, endPoint, certificate, certificateChain);
	}
}
