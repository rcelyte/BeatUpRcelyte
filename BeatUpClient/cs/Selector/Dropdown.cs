using static System.Linq.Enumerable;

static partial class BeatUpClient {
	class ServerDropdown : DropdownSettingsController {
		struct Entry {
			public string? name, status;
		}
		static bool EnableOfficial = true;
		Entry[] options = null!;
		Entry ephemeral = new Entry();
		int ephemeralIndex = -1;
		internal UnityEngine.GameObject? addButton = null, editButton = null;
		internal UnityEngine.GameObject? removeButton = null, saveButton = null;
		public ServerDropdown() {
			_dropdown = GetComponent<HMUI.SimpleTextDropdown>();
			_dropdown._modalView._dismissPanelAnimation._duration = 0; // Animations will break the UI if the player clicks too fast
			BeatUpClient_Config.Instance.Servers.TryGetValue(customServerHostName, out string? status);
			SetServer(customServerHostName, status);
		}
		protected override bool GetInitValues(out int idx, out int numberOfElements) {
			idx = 0;
			for(int i = System.Convert.ToInt32(EnableOfficial), count = options.Length; i < count; ++i) {
				if(options[i].name != customServerHostName.value)
					continue;
				idx = i;
				break;
			}
			numberOfElements = options.Length;
			RefreshEphemeralUI(idx == ephemeralIndex);
			Log.Debug($"ServerDropdown.GetInitValues(idx={idx}, numberOfElements={numberOfElements})");
			return true;
		}
		protected override void ApplyValue(int idx) {
			RefreshEphemeralUI(idx == ephemeralIndex);
			Entry newValue = options[idx];
			if(!SetNetworkConfig(newValue.name, newValue.status))
				return;
			_dropdown.Hide(false);
			ReenterModeSelection();
		}
		protected override string TextForValue(int idx) =>
			options[idx].name ?? "Official Server";
		public void SetServer(string name, string? status) {
			Log.Debug($"ServerDropdown.SetServer(\"{name}\", \"{status}\")");
			System.Collections.Generic.IEnumerable<Entry> addrs = BeatUpClient_Config.Instance.Servers.Select(s => new Entry {name = s.Key, status = s.Value});
			if(!(string.IsNullOrEmpty(name) || BeatUpClient_Config.Instance.Servers.ContainsKey(name)))
				ephemeral = new Entry {name = name, status = status};
			else if(name == ephemeral.name)
				ephemeral = new Entry();
			ephemeralIndex = -1;
			if(!string.IsNullOrEmpty(ephemeral.name)) {
				ephemeralIndex = System.Convert.ToInt32(EnableOfficial);
				addrs = addrs.Prepend(ephemeral);
			}
			if(EnableOfficial)
				addrs = addrs.Prepend(new Entry());
			options = addrs.ToArray();
			Refresh(false);
		}
		internal void ClearEphemeral() {
			ephemeral = new Entry();
			if(BeatUpClient_Config.Instance.Servers.TryGetValue(customServerHostName, out string? statusUrl))
				SetServer(customServerHostName, statusUrl);
			else
				SetServer(string.Empty, null);
			Refresh(true);
		}
		internal void SaveEphemeral() {
			if(NullableStringHelper.IsNullOrEmpty(ephemeral.name))
				return;
			BeatUpClient_Config.Instance.Servers[ephemeral.name] = ephemeral.status;
			BeatUpClient_Config.Instance.Changed();
			SetServer(ephemeral.name, ephemeral.status);
		}
		void RefreshEphemeralUI(bool isEphemeral) {
			_dropdown._text.color = isEphemeral ? new UnityEngine.Color(1, 1, 0.1956522f, 1) : UnityEngine.Color.white;
			addButton?.SetActive(!isEphemeral);
			editButton?.SetActive(!isEphemeral);
			removeButton?.SetActive(isEphemeral);
			saveButton?.SetActive(isEphemeral);
		}
	}

	static UnityEngine.RectTransform CreateServerDropdown(UnityEngine.Transform parent, string name, IValue<string> value, System.Collections.Generic.IEnumerable<string> options) {
		UnityEngine.GameObject simpleDropdownTemplate = UnityEngine.Resources.FindObjectsOfTypeAll<HMUI.SimpleTextDropdown>().First(dropdown => dropdown.GetComponents(typeof(UnityEngine.Component)).Length == 2).gameObject;
		UnityEngine.GameObject gameObject = UI.CreateElement(simpleDropdownTemplate, parent, name);
		gameObject.AddComponent<ServerDropdown>();
		gameObject.SetActive(true);
		return (UnityEngine.RectTransform)gameObject.transform;
	}
}
