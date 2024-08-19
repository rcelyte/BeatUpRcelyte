static partial class BeatUpClient {
	struct VanillaConfig : INetworkConfig {
		public int maxPartySize {get; set;}
		public int discoveryPort {get;}
		public int partyPort {get;}
		public int multiplayerPort {get;}
		public DnsEndPoint masterServerEndPoint {get; set;}
		public string multiplayerStatusUrl {get; set;}
		public string quickPlaySetupUrl {get;}
		public string graphUrl {get; set;}
		public string graphAccessToken {get;}
		public bool forceGameLift {get; set;}
		public ServiceEnvironment serviceEnvironment {get;}
		public string appId {get;}
		public VanillaConfig(NetworkConfigSO from) => // Bypass all getters other mods might patch
			(this.maxPartySize, this.discoveryPort, this.partyPort, this.multiplayerPort, this.masterServerEndPoint, this.multiplayerStatusUrl, this.quickPlaySetupUrl, this.graphUrl, this.graphAccessToken, this.forceGameLift, this.serviceEnvironment, this.appId) = 
				(/*from._maxPartySize*/5, from._discoveryPort, from._partyPort, from._multiplayerPort, new DnsEndPoint(from._masterServerHostName, from._masterServerPort), from._multiplayerStatusUrl, from._quickPlaySetupUrl, from.graphUrl, from.graphAccessToken, from._forceGameLift, from._serviceEnvironment, from.appId);
	}

	static VanillaConfig officialConfig;
	static void NetworkConfigSetup(NetworkConfigSO networkConfig) {
		if(officialConfig.masterServerEndPoint == null)
			officialConfig = new VanillaConfig(networkConfig);
	}

	static bool SetNetworkConfig(string hostname, string statusUrl) {
		Resolve<SettingsManager>()!.settings.customServer.hostName = hostname;
		CustomNetworkConfig? customNetworkConfig = Resolve<CustomNetworkConfig>();
		if(customNetworkConfig == null)
			return false;
		VanillaConfig newConfig = officialConfig;
		if(!string.IsNullOrEmpty(hostname)) {
			string[] splitHostname = hostname.ToLower().Split(new[] {':'});
			int port = officialConfig.masterServerEndPoint.port;
			if(splitHostname.Length >= 2)
				if(int.TryParse(splitHostname[1], out int customPort) && customPort > 0)
					port = customPort;
			newConfig.maxPartySize = 254;
			newConfig.masterServerEndPoint = new DnsEndPoint(splitHostname[0], port);
			newConfig.multiplayerStatusUrl = string.IsNullOrEmpty(statusUrl) ? "https://status." + splitHostname[0] : statusUrl;
			newConfig.graphUrl = hostname;
			newConfig.forceGameLift = hostname.StartsWith("http://") || hostname.StartsWith("https://");
			if(GameLiftRequired && !newConfig.forceGameLift) {
				newConfig.graphUrl = newConfig.multiplayerStatusUrl;
				newConfig.forceGameLift = true;
			}
		}
		string oldGraphUrl = customNetworkConfig.graphUrl, oldStatusUrl = customNetworkConfig.multiplayerStatusUrl;
		typeof(CustomNetworkConfig).GetConstructors()[0].Invoke(customNetworkConfig, new object[] {newConfig, "", 0, true});
		HarmonyLib.AccessTools.Field(typeof(CustomNetworkConfig), "<forceGameLift>k__BackingField").SetValue(customNetworkConfig, (bool)newConfig.forceGameLift);
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

	[Detour(typeof(ClientCertificateValidator), nameof(ClientCertificateValidator.ValidateCertificateChainInternal))]
	static void ClientCertificateValidator_ValidateCertificateChainInternal(ClientCertificateValidator self, DnsEndPoint endPoint, System.Security.Cryptography.X509Certificates.X509Certificate2 certificate, byte[][] certificateChain) {
		if(Resolve<CustomNetworkConfig>() is CustomNetworkConfig networkConfig && networkConfig.graphUrl != officialConfig.graphUrl)
			return;
		Base(self, endPoint, certificate, certificateChain);
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
