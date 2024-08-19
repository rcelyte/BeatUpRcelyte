using static System.Linq.Enumerable;

static partial class BeatUpClient {
	class ServerDropdown : DropdownSettingsController {
		const bool EnableOfficial = true;

		readonly struct Entry {
			public readonly string name, status;
			public Entry(string name, string status) =>
				(this.name, this.status) = (name, status);
			public bool Existing() => (EnableOfficial && string.IsNullOrEmpty(name)) || BeatUpClient_Config.Instance.Servers.ContainsKey(name);
		}

		readonly MultiplayerModeSelectionFlowCoordinator flowCoordinator;
		Entry? ephemeralEntry = null;
		Entry[] options = new Entry[0];
		readonly UnityEngine.UI.Button editButton;
		readonly UnityEngine.GameObject addButton, saveButton, removeButton;
		public ServerDropdown() {
			_dropdown = GetComponent<HMUI.SimpleTextDropdown>();
			_dropdown._modalView._dismissPanelAnimation._duration = 0; // Animations will break the UI if the player clicks too fast
			flowCoordinator = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerModeSelectionFlowCoordinator>()[0];
			ephemeralEntry = new Entry(Resolve<SettingsManager>()!.settings.customServer.hostName, "");

			UnityEngine.GameObject colorSchemeButton = UnityEngine.Resources.FindObjectsOfTypeAll<ColorsOverrideSettingsPanelController>()[0]._editColorSchemeButton.gameObject;
			UnityEngine.GameObject okButton = UnityEngine.Resources.FindObjectsOfTypeAll<EditColorSchemeController>()[0]._closeButton.gameObject;
			UnityEngine.Sprite[] sprites = UnityEngine.Resources.FindObjectsOfTypeAll<UnityEngine.Sprite>();

			editButton = UI.CreateButtonFrom(colorSchemeButton, transform, "EditServer", () =>
				EditServerViewController.Instance.TryPresent(true)).gameObject.GetComponent<UnityEngine.UI.Button>();
			editButton.interactable = false;
			UnityEngine.RectTransform addButton = UI.CreateButtonFrom(colorSchemeButton, transform, "AddServer", () =>
				EditServerViewController.Instance.TryPresent(false));
			addButton.Find("Icon").GetComponent<HMUI.ImageView>().sprite = sprites.FirstOrDefault(sprite => sprite.name == "AddIcon");
			this.addButton = addButton.gameObject;

			UnityEngine.RectTransform smallButton = (UnityEngine.RectTransform)colorSchemeButton.transform;
			UnityEngine.GameObject smallIcon = smallButton.Find("Icon").gameObject;
			UnityEngine.GameObject CreateShinyButton(float iconScale, string name, UnityEngine.Sprite? sprite, System.Action callback) {
				UnityEngine.RectTransform button = UI.CreateButtonFrom(okButton, transform, name, callback);
				UnityEngine.Object.Destroy(button.Find("Content").gameObject);
				(button.pivot, button.sizeDelta) = (smallButton.pivot, smallButton.sizeDelta - new UnityEngine.Vector2(1, 0));
				UnityEngine.GameObject icon = UnityEngine.Object.Instantiate(smallIcon, button);
				icon.transform.localScale = new UnityEngine.Vector3(iconScale, iconScale, 1);
				icon.GetComponent<HMUI.ImageView>().sprite = sprite;
				return button.gameObject;
			}
			saveButton = CreateShinyButton(.75f, "SaveServer", sprites.FirstOrDefault(sprite => sprite.name == "DownloadIcon"), SaveEphemeral);
			removeButton = CreateShinyButton(.65f, "RemoveServer", sprites.FirstOrDefault(sprite => sprite.name == "CloseIcon"), ClearEphemeral);
		}
		void Awake() {
			editButton.transform.localPosition = new UnityEngine.Vector3(52, 0, 0);
			addButton.transform.localPosition = new UnityEngine.Vector3(-40, 0, 0);
			saveButton.transform.localPosition = new UnityEngine.Vector3(51.5f, 0, 0);
			removeButton.transform.localPosition = new UnityEngine.Vector3(-40.5f, 0, 0);

			onNetworkConfigChanged += SetServer;
			BeatUpClient_Config.onReload += () => Refresh(false);
		}
		void OnDestroy() =>
			onNetworkConfigChanged -= SetServer;
		protected override bool GetInitValues(out int idx, out int numberOfElements) {
			System.Collections.Generic.IEnumerable<Entry> addrs = BeatUpClient_Config.Instance.Servers.Select(s => new Entry(s.Key, s.Value ?? ""));
			if(ephemeralEntry is Entry entry) {
				if(entry.Existing())
					ephemeralEntry = null;
				else
					addrs = addrs.Prepend(entry);
			}
			if(EnableOfficial)
				addrs = addrs.Prepend(new Entry("", ""));
			options = addrs.ToArray();

			idx = 0;
			for(int i = 0, count = options.Length; i < count; ++i) {
				if(options[i].name != Resolve<SettingsManager>()!.settings.customServer.hostName)
					continue;
				idx = i;
				break;
			}
			numberOfElements = options.Length;
			RefreshStyle(idx);
			Log.Debug($"ServerDropdown.GetInitValues(idx={idx}, numberOfElements={numberOfElements})");
			return true;
		}
		protected override void ApplyValue(int idx) {
			RefreshStyle(idx);
			onNetworkConfigChanged -= SetServer;
			bool changed = UpdateNetworkConfig(options[idx].name, options[idx].status);
			onNetworkConfigChanged += SetServer;
			if(!changed)
				return;
			_dropdown.Hide(false);
			ReenterModeSelection(flowCoordinator);
		}
		protected override string TextForValue(int idx) =>
			string.IsNullOrEmpty(options[idx].name) ? "Official Server" : options[idx].name;
		public void SetServer(string name, string status) {
			Log.Debug($"ServerDropdown.SetServer(\"{name}\", \"{status}\")");
			Entry entry = new Entry(name, status);
			if(!entry.Existing())
				ephemeralEntry = entry;
			Refresh(false);
		}
		void ClearEphemeral() {
			ephemeralEntry = null;
			Refresh(true);
		}
		void SaveEphemeral() {
			if(string.IsNullOrEmpty(ephemeralEntry?.name))
				return;
			Entry entry = ephemeralEntry.GetValueOrDefault();
			BeatUpClient_Config.Instance.Servers[entry.name] = entry.status;
			BeatUpClient_Config.Instance.Changed();
		}
		void RefreshStyle(int idx) {
			bool existing = options[idx].Existing();
			_dropdown._text.color = existing ? UnityEngine.Color.white : new UnityEngine.Color(1, 1, 0.1956522f, 1);
			editButton.gameObject.SetActive(existing);
			addButton.SetActive(existing);
			saveButton.SetActive(!existing);
			removeButton.SetActive(!existing);
			editButton.interactable = !string.IsNullOrEmpty(options[idx].name);
		}

		public static UnityEngine.RectTransform Create(UnityEngine.Transform parent, string name) {
			UnityEngine.GameObject template = UnityEngine.Resources.FindObjectsOfTypeAll<HMUI.SimpleTextDropdown>()
				.First(dropdown => dropdown.GetComponents(typeof(UnityEngine.Component)).Length == 2).gameObject;
			UnityEngine.GameObject gameObject = UI.CreateElement(template, parent, name);
			gameObject.AddComponent<ServerDropdown>();
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
	}
}
