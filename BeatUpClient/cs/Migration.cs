using static System.Linq.Enumerable;

static class BeatUpClient_Migration {
	static System.Threading.Tasks.Task<System.Collections.Generic.List<(string, string)>>? legacyHostnameMapping = null;
	static void TryFinishMigration() {
		if(legacyHostnameMapping?.IsCompleted != true)
			return;
		System.Collections.Generic.List<(string, string)> hostnameMap = legacyHostnameMapping.Result;
		legacyHostnameMapping = null;
		if(hostnameMap.Count == 0)
			return;
		(string? graphUrl, string? statusUrl) newHostname = default;
		foreach((string hostname, string graphUrl) in hostnameMap) {
			if(!BeatUpClient_Config.Instance.Servers.TryGetValue(hostname, out string? statusUrl))
				continue;
			BeatUpClient_Config.Instance.Servers.TryAdd(graphUrl, statusUrl);
			BeatUpClient_Config.Instance.Servers.Remove(hostname);
			if(hostname == BeatUpClient.customServerHostName.value)
				newHostname = (graphUrl, statusUrl);
		}
		BeatUpClient_Config.Instance.Changed();
		if(newHostname.graphUrl == null)
			return;
		BeatUpClient.UpdateNetworkConfig(newHostname.graphUrl, newHostname.statusUrl ?? "");
	}

	[BeatUpClient.Detour(typeof(MainFlowCoordinator), nameof(MainFlowCoordinator.HandleMainMenuViewControllerDidFinish))]
	static void MainFlowCoordinator_HandleMainMenuViewControllerDidFinish(MainFlowCoordinator self, MainMenuViewController viewController, MainMenuViewController.MenuButton subMenuType) {
		if(subMenuType != MainMenuViewController.MenuButton.Multiplayer || legacyHostnameMapping?.IsCompleted != false) {
			TryFinishMigration();
			BeatUpClient.Base(self, viewController, subMenuType);
			return;
		}
		bool dismissed = false;
		self._simpleDialogPromptViewController.Init("BeatTogether Migration", "Migrating legacy hostnames...", Polyglot.Localization.Get("BUTTON_CANCEL"), buttonNumber => {
			dismissed = true;
			self.DismissViewController(self._simpleDialogPromptViewController, HMUI.ViewController.AnimationDirection.Vertical);
		});
		self.PresentViewController(self._simpleDialogPromptViewController, async() => {
			await legacyHostnameMapping;
			if(dismissed)
				return;
			TryFinishMigration();
			self.DismissViewController(self._simpleDialogPromptViewController, HMUI.ViewController.AnimationDirection.Vertical, () =>
				self.HandleMainMenuViewControllerDidFinish(viewController, MainMenuViewController.MenuButton.Multiplayer));
		});
	}

	static async System.Threading.Tasks.Task<System.Collections.Generic.List<(string, string)>> MigrateLegacyHostnames() {
		System.Collections.Generic.List<(string, string)> hostnameMap = new();
		await System.Threading.Tasks.Task.WhenAll(BeatUpClient_Config.Instance.Servers.Keys
			.Where(hostname => !(hostname.StartsWith("http://") || hostname.StartsWith("https://")))
			.Select(async(string hostname) => {
				try {
					string[] splitHostname = hostname.ToLower().Split(new[] {':'});
					if(splitHostname.Length == 0)
						return;
					System.Net.Http.HttpClient httpClient = new();
					System.Net.Http.StringContent request = new("{}", System.Text.Encoding.UTF8, "application/json");
					string graphUrl = $"http://{splitHostname[0]}:8989";
					httpClient.Timeout = System.TimeSpan.FromSeconds(5);
					System.Net.Http.HttpResponseMessage message = await httpClient.PostAsync((new System.UriBuilder(graphUrl) {Path = "beat_saber_get_multiplayer_instance"}).Uri, request);
					if(!message.IsSuccessStatusCode)
						return;
					string resp = await message.Content.ReadAsStringAsync();
					if(Newtonsoft.Json.JsonConvert.DeserializeObject<BGNet.Core.GameLift.GetMultiplayerInstanceResponse>(resp).errorCode != MultiplayerPlacementErrorCode.AuthenticationFailed)
						return;
					BeatUpClient.Log.Info($"Migrating hostname `{hostname}` -> `{graphUrl}`");
					hostnameMap.Add((hostname, graphUrl));
				} catch(System.Exception) {}
			}));
		return hostnameMap;
	}

	public static void Init() {
		legacyHostnameMapping = MigrateLegacyHostnames();
		uint patchCount = 0;
		BeatUpClient.GatherMethods(typeof(BeatUpClient_Migration), ref patchCount)();
	}
}
