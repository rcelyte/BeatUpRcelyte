using static System.Linq.Enumerable;

static partial class BeatUpClient {
	static class UI {
		public class ToggleSetting : SwitchSettingsController {
			public IValue<bool> setting = null!;
			public ToggleSetting() {
				_toggle = gameObject.transform.GetChild(1).gameObject.GetComponent<UnityEngine.UI.Toggle>();
				_toggle.onValueChanged.RemoveAllListeners();
				BeatUpClient_Config.onReload += OnEnable;
			}
			protected override bool GetInitValue() => setting.value;
			protected override void ApplyValue(bool value) =>
				setting.value = value;
		}
		public class ValuePickerSetting : ListSettingsController {
			public byte[] options = null!;
			public IValue<float> setting = null!;
			public int startIdx = 1;
			public ValuePickerSetting() {
				_stepValuePicker = gameObject.transform.GetChild(1).gameObject.GetComponent<StepValuePicker>();
				BeatUpClient_Config.onReload += OnEnable;
			}
			protected override bool GetInitValues(out int idx, out int numberOfElements) {
				idx = System.Array.LastIndexOf(options, (byte)(setting.value * 4));
				if(idx == 0)
					startIdx = 0;
				else
					--idx;
				numberOfElements = options.Length - startIdx;
				return true;
			}
			protected override void ApplyValue(int idx) =>
				setting.value = options[idx + startIdx] / 4.0f;
			protected override string TextForValue(int idx) => $"{options[idx + startIdx] / 4.0f}";
		}
		internal static UnityEngine.GameObject CreateElement(UnityEngine.GameObject template, UnityEngine.Transform parent, string name) {
			UnityEngine.GameObject gameObject = UnityEngine.Object.Instantiate(template, parent);
			gameObject.name = "BeatUpClient_" + name;
			gameObject.SetActive(false);
			return gameObject;
		}
		static UnityEngine.GameObject CreateElementWithText(UnityEngine.GameObject template, UnityEngine.Transform parent, string name, string headerKey) {
			UnityEngine.GameObject gameObject = CreateElement(template, parent, name);
			((UnityEngine.RectTransform)gameObject.transform).sizeDelta = new UnityEngine.Vector2(90, ((UnityEngine.RectTransform)gameObject.transform).sizeDelta.y);
			UnityEngine.GameObject nameText = gameObject.transform.Find("NameText").gameObject;
			nameText.GetComponent<Polyglot.LocalizedTextMeshProUGUI>().Key = headerKey;
			return gameObject;
		}
		public static UnityEngine.RectTransform CreateToggle(UnityEngine.Transform parent, string name, string headerKey, IValue<bool> value) {
			UnityEngine.GameObject toggleTemplate = UnityEngine.Resources.FindObjectsOfTypeAll<UnityEngine.UI.Toggle>().Select(x => x.transform.parent.gameObject).First(p => p.name == "Fullscreen");
			UnityEngine.GameObject gameObject = CreateElementWithText(toggleTemplate, parent, name, headerKey);
			UnityEngine.Object.Destroy(gameObject.GetComponent<BoolSettingsController>());
			ToggleSetting toggleSetting = gameObject.AddComponent<ToggleSetting>();
			toggleSetting.setting = value;
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
		public static UnityEngine.RectTransform CreateValuePicker(UnityEngine.Transform parent, string name, string headerKey, IValue<float> value, byte[] options) {
			UnityEngine.GameObject pickerTemplate = UnityEngine.Resources.FindObjectsOfTypeAll<StepValuePicker>().Select(x => x.transform.parent.gameObject).First(p => p.name == "MaxNumberOfPlayers");
			UnityEngine.GameObject gameObject = CreateElementWithText(pickerTemplate, parent, name, headerKey);
			UnityEngine.Object.Destroy(gameObject.GetComponent<FormattedFloatListSettingsController>());
			ValuePickerSetting valuePickerSetting = gameObject.AddComponent<ValuePickerSetting>();
			valuePickerSetting.options = options;
			valuePickerSetting.setting = value;
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
		public static UnityEngine.RectTransform CreateButtonFrom(UnityEngine.GameObject template, UnityEngine.Transform parent, string name, System.Action callback) {
			UnityEngine.GameObject gameObject = CreateElement(template, parent, name);
			UnityEngine.UI.Button button = gameObject.GetComponent<UnityEngine.UI.Button>();
			button.onClick.RemoveAllListeners();
			button.onClick.AddListener(callback.Invoke);
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
		public static UnityEngine.RectTransform CreateClone(UnityEngine.GameObject template, UnityEngine.Transform parent, string name, int index = -1) {
			UnityEngine.GameObject gameObject = CreateElement(template, parent, name);
			if(index >= 0)
				gameObject.transform.SetSiblingIndex(index);
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
	}
}
