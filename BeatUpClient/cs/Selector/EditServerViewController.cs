using static System.Linq.Enumerable;

static partial class BeatUpClient {
	public class EditServerViewController : HMUI.ViewController {
		public static EditServerViewController Instance = null!;
		internal MainFlowCoordinator mainFlowCoordinator = null!;
		MultiplayerModeSelectionFlowCoordinator? flowCoordinator = null;
		bool edit = false;
		VRUIControls.VRInputModule vrInputModule = null!;
		HMUI.InputFieldView editHostnameTextbox = null!;
		HMUI.InputFieldView editStatusTextbox = null!;
		HMUI.CurvedTextMeshPro editStatusPlaceholder = null!;
		HMUI.UIKeyboard keyboard = null!;
		UnityEngine.UI.Button cancelButton = null!;
		string? activeKey = null;

		static HMUI.InputFieldView CreateTextbox(UnityEngine.Transform parent, float width, float y, string name, int index, System.Action<HMUI.InputFieldView> callback, string? placeholderKey = null) {
			UnityEngine.GameObject textboxTemplate = UnityEngine.Resources.FindObjectsOfTypeAll<EnterPlayerGuestNameViewController>()[0]._nameInputFieldView.gameObject;
			UnityEngine.GameObject gameObject = UI.CreateElement(textboxTemplate, parent, name);
			HMUI.InputFieldView inputField = gameObject.GetComponent<HMUI.InputFieldView>();
			inputField._textLengthLimit = 180;
			inputField.onValueChanged = new HMUI.InputFieldView.InputFieldChanged();
			inputField.onValueChanged.AddListener(callback.Invoke);
			BGLib.Polyglot.LocalizedTextMeshProUGUI localizedText = inputField._placeholderText.GetComponent<BGLib.Polyglot.LocalizedTextMeshProUGUI>();
			localizedText.enabled = (placeholderKey == null);
			localizedText.Key = placeholderKey ?? string.Empty;
			UnityEngine.RectTransform transform = (UnityEngine.RectTransform)gameObject.transform;
			transform.sizeDelta = new UnityEngine.Vector2(width, transform.sizeDelta.y);
			transform.localPosition = new UnityEngine.Vector3(0, y, 0);
			transform.SetSiblingIndex(index);
			gameObject.SetActive(true);
			return inputField;
		}

		static UnityEngine.GameObject AddKey(HMUI.UIKeyboard keyboard, bool numpad, int row, int col, UnityEngine.KeyCode keyCode, char charCode, bool canBeUppercase = false) {
			UnityEngine.Transform parent = keyboard.transform.Find(numpad ? "Numpad" : "Letters").GetChild(row);
			UnityEngine.Transform refKey = parent.GetChild(numpad ? 0 : 1);
			UnityEngine.GameObject key = UnityEngine.Object.Instantiate(refKey.gameObject, parent);
			key.name = "" + keyCode;
			if(col < 0)
				col += parent.childCount;
			key.transform.SetSiblingIndex(col);
			if(numpad) {
				parent.GetComponent<UnityEngine.UI.HorizontalLayoutGroup>().enabled = true;
			} else {
				UnityEngine.Vector3 refPosition = refKey.localPosition;
				for(int i = col, count = parent.childCount; i < count; ++i)
					parent.GetChild(i).localPosition = new UnityEngine.Vector3(refPosition.x + i * 7 - 7, refPosition.y, refPosition.z);
			}
			HMUI.UIKeyboardKey keyboardKey = key.GetComponent<HMUI.UIKeyboardKey>();
			keyboardKey._keyCode = keyCode;
			keyboardKey._overrideText = $"{charCode}";
			keyboardKey._canBeUppercase = canBeUppercase;
			keyboardKey.Awake();
			keyboard._buttonBinder.AddBinding(key.GetComponent<HMUI.NoTransitionsButton>(), () =>
				keyboard.keyWasPressedEvent(charCode));
			return key;
		}

		public static EditServerViewController Create(string name, UnityEngine.Transform parent) {
			UnityEngine.GameObject gameObject = UnityEngine.Object.Instantiate(UnityEngine.Resources.FindObjectsOfTypeAll<ServerCodeEntryViewController>()[0].gameObject, parent);
			gameObject.name = name;
			UnityEngine.Object.Destroy(gameObject.GetComponent<ServerCodeEntryViewController>());
			UnityEngine.Object.Destroy(gameObject.GetComponentInChildren<HMUI.InputFieldView>().gameObject);
			EditServerViewController viewController = gameObject.AddComponent<EditServerViewController>();
			viewController.rectTransform.anchorMin = new UnityEngine.Vector2(0f, 0f);
			viewController.rectTransform.anchorMax = new UnityEngine.Vector2(1f, 1f);
			viewController.rectTransform.sizeDelta = new UnityEngine.Vector2(0f, 0f);
			viewController.rectTransform.anchoredPosition = new UnityEngine.Vector2(0f, 0f);
			viewController.vrInputModule = UnityEngine.Resources.FindObjectsOfTypeAll<VRUIControls.VRInputModule>()[0];

			UnityEngine.Transform Wrapper = gameObject.transform.GetChild(0);
			viewController.editHostnameTextbox = CreateTextbox(Wrapper, 80, -1.5f, "EditHostname", 1, viewController.RefreshStatusPlaceholder, "BEATUP_ENTER_HOSTNAME");
			viewController.editStatusTextbox = CreateTextbox(Wrapper, 80, -8.5f, "EditStatus", 2, viewController.RefreshStatusPlaceholder);
			viewController.editStatusPlaceholder = viewController.editStatusTextbox._placeholderText.GetComponent<HMUI.CurvedTextMeshPro>();
			viewController.cancelButton = Wrapper.Find("Buttons/CancelButton").GetComponent<HMUI.NoTransitionsButton>();

			viewController.keyboard = gameObject.GetComponentInChildren<HMUI.UIKeyboard>();
			foreach(UnityEngine.RectTransform tr in new[] {viewController.keyboard.transform.parent, viewController.keyboard.transform})
				tr.sizeDelta = new UnityEngine.Vector2(tr.sizeDelta.x + 7, tr.sizeDelta.y);
			AddKey(viewController.keyboard, false, 0, -1, UnityEngine.KeyCode.Underscore, '_');
			AddKey(viewController.keyboard, false, 1, -1, UnityEngine.KeyCode.Colon, ':');
			AddKey(viewController.keyboard, false, 2, -2, UnityEngine.KeyCode.Slash, '/');
			UnityEngine.GameObject dot = AddKey(viewController.keyboard, true, 3, -1, UnityEngine.KeyCode.Period, '.');
			((UnityEngine.RectTransform)dot.transform.parent.GetChild(0)).sizeDelta = new UnityEngine.Vector2(14, 7);

			gameObject.SetActive(false);
			return viewController;
		}

		protected override void DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
			if(firstActivation)
				base.buttonBinder.AddBinding(cancelButton, () => Dismiss());
			string? hostName = Resolve<SettingsManager>()!.settings.customServer.hostName ?? string.Empty, status = null;
			activeKey = (edit && BeatUpClient_Config.Instance.Servers.TryGetValue(hostName, out status)) ? hostName : null;
			editHostnameTextbox.SetText(activeKey ?? string.Empty);
			editHostnameTextbox.onValueChanged.Invoke(editHostnameTextbox);
			editStatusTextbox.SetText(status ?? string.Empty);
			RefreshStatusPlaceholder(editStatusTextbox);
			editHostnameTextbox.ActivateKeyboard(keyboard);
			editHostnameTextbox.UpdateClearButton();
			keyboard.okButtonWasPressedEvent += HandleOkButtonWasPressed;
			vrInputModule.onProcessMousePressEvent += ProcessMousePress;
		}

		protected override void DidDeactivate(bool removedFromHierarchy, bool screenSystemDisabling) {
			vrInputModule.onProcessMousePressEvent -= ProcessMousePress;
			keyboard.okButtonWasPressedEvent -= HandleOkButtonWasPressed;
			editStatusTextbox.DeactivateKeyboard(keyboard);
			editHostnameTextbox.DeactivateKeyboard(keyboard);
			flowCoordinator = null;
		}

		void ProcessMousePress(UnityEngine.GameObject gameObject) {
			HMUI.InputFieldView inputField = gameObject.GetComponent<HMUI.InputFieldView>();
			if(inputField == editHostnameTextbox) {
				editStatusTextbox.DeactivateKeyboard(keyboard);
				editHostnameTextbox.ActivateKeyboard(keyboard);
				editHostnameTextbox.UpdateClearButton();
			} else if(inputField == editStatusTextbox) {
				editHostnameTextbox.DeactivateKeyboard(keyboard);
				editStatusTextbox.ActivateKeyboard(keyboard);
				editStatusTextbox.UpdateClearButton();
			}
		}

		void RefreshStatusPlaceholder(HMUI.InputFieldView textbox) {
			string text = editHostnameTextbox.text.Split(new[] {':'})[0];
			bool enable = !string.IsNullOrEmpty(text);
			if(enable != editStatusTextbox.gameObject.activeSelf) {
				editStatusTextbox.gameObject.SetActive(enable);
				if(!enable)
					editStatusTextbox.DeactivateKeyboard(keyboard);
			}
			if(enable)
				editStatusPlaceholder.text = "https://status." + text;
		}

		public void TryPresent(bool edit) {
			if(this.flowCoordinator == null && mainFlowCoordinator.childFlowCoordinator is MultiplayerModeSelectionFlowCoordinator modeSelection) {
				this.flowCoordinator = modeSelection;
				this.edit = edit;
				modeSelection.PresentViewController(this, null, HMUI.ViewController.AnimationDirection.Vertical, false);
				modeSelection.SetTitle(BGLib.Polyglot.Localization.Get(edit ? "BEATUP_EDIT_SERVER" : "BEATUP_ADD_SERVER"), HMUI.ViewController.AnimationType.In);
				modeSelection._screenSystem.SetBackButton(false, true);
			}
		}

		public void Dismiss(bool immediately = false) =>
			flowCoordinator?.DismissViewController(this, HMUI.ViewController.AnimationDirection.Vertical, null, immediately);

		void HandleOkButtonWasPressed() {
			if(activeKey != null)
				BeatUpClient_Config.Instance.Servers.Remove(activeKey);
			string? hostname = editHostnameTextbox.text;
			string status = string.Empty;
			if(hostname.Length < 1 || hostname.IndexOf(':') == 0)
				hostname = null;
			else if(editStatusTextbox.text.Length >= 1)
				status = editStatusTextbox.text;
			if(hostname != null)
				BeatUpClient_Config.Instance.Servers[hostname] = status;
			BeatUpClient_Config.Instance.Changed();
			UpdateNetworkConfig(hostname ?? string.Empty, status);
			MultiplayerModeSelectionFlowCoordinator? lastFlowCoordinator = flowCoordinator;
			Dismiss(true);
			ReenterModeSelection(lastFlowCoordinator);
		}
	}
}
