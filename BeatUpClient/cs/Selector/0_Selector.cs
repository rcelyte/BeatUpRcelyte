using static System.Linq.Enumerable;

static partial class BeatUpClient {
	static void ReenterModeSelection() {
		if(mainFlowCoordinator?.childFlowCoordinator is MultiplayerModeSelectionFlowCoordinator multiplayerModeSelectionFlowCoordinator) {
			mainFlowCoordinator.DismissFlowCoordinator(multiplayerModeSelectionFlowCoordinator, HMUI.ViewController.AnimationDirection.Horizontal, () => {
				mainFlowCoordinator.PresentMultiplayerModeSelectionFlowCoordinatorWithDisclaimerAndAvatarCreator();
			}, true);
		}
	}

	static EditServerViewController editServerViewController = null!;
	static UnityEngine.UI.Button editServerButton = null!;
	static void SelectorSetup() {
		UnityEngine.Transform CreateServerFormView = UnityEngine.Resources.FindObjectsOfTypeAll<CreateServerFormController>()[0].transform;
		BeatUpServerUI = new[] {
			UI.CreateValuePicker(CreateServerFormView, "CountdownDuration", "BEATUP_COUNTDOWN_DURATION", new Property<float>(BeatUpClient_Config.Instance, nameof(BeatUpClient_Config.CountdownDuration)), new byte[] {(byte)(BeatUpClient_Config.Instance.CountdownDuration * 4), 0, 12, 20, 32, 40, 60}).gameObject,
			UI.CreateToggle(CreateServerFormView, "SkipResults", "BEATUP_SKIP_RESULTS_PYRAMID", new Property<bool>(BeatUpClient_Config.Instance, nameof(BeatUpClient_Config.SkipResults))).gameObject,
			UI.CreateToggle(CreateServerFormView, "PerPlayerDifficulty", "BEATUP_PER_PLAYER_DIFFICULTY", new Property<bool>(BeatUpClient_Config.Instance, nameof(BeatUpClient_Config.PerPlayerDifficulty))).gameObject,
			UI.CreateToggle(CreateServerFormView, "PerPlayerModifiers", "BEATUP_PER_PLAYER_MODIFIERS", new Property<bool>(BeatUpClient_Config.Instance, nameof(BeatUpClient_Config.PerPlayerModifiers))).gameObject,
		};
		{
			UnityEngine.Transform parent = CreateServerFormView.parent;
			foreach(UnityEngine.RectTransform child in parent) {
				if(child.GetComponents<UnityEngine.UI.ILayoutElement>().Length > 0)
					continue;
				UnityEngine.UI.LayoutElement layout = child.gameObject.AddComponent<UnityEngine.UI.LayoutElement>();
				layout.preferredHeight = child.sizeDelta.y;
			}
			UnityEngine.UI.VerticalLayoutGroup parentGroup = parent.gameObject.GetComponent<UnityEngine.UI.VerticalLayoutGroup>();
			parentGroup.enabled = true;
			parentGroup.childControlHeight = true;
			CreateServerFormView.gameObject.GetComponent<UnityEngine.UI.VerticalLayoutGroup>().enabled = true;
			CreateServerFormView.gameObject.GetComponent<UnityEngine.UI.ContentSizeFitter>().enabled = true;
		}

		mainFlowCoordinator = UnityEngine.Resources.FindObjectsOfTypeAll<MainFlowCoordinator>()[0];
		MultiplayerModeSelectionViewController multiplayerModeSelectionViewController = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerModeSelectionViewController>()[0];
		TMPro.TextMeshProUGUI customServerEndPointText = multiplayerModeSelectionViewController._customServerEndPointText;
		UnityEngine.GameObject colorSchemeButton = UnityEngine.Resources.FindObjectsOfTypeAll<ColorsOverrideSettingsPanelController>()[0]._editColorSchemeButton.gameObject;
		UnityEngine.GameObject okButton = UnityEngine.Resources.FindObjectsOfTypeAll<EditColorSchemeController>()[0]._closeButton.gameObject;
		UnityEngine.Sprite[] sprites = UnityEngine.Resources.FindObjectsOfTypeAll<UnityEngine.Sprite>();

		customServerEndPointText.enabled = false;
		editServerViewController ??= EditServerViewController.Create("BeatUpClient_EditServerView", multiplayerModeSelectionViewController.transform.parent);
		UnityEngine.RectTransform server = CreateServerDropdown(customServerEndPointText.transform, "Selector", customServerHostName, BeatUpClient_Config.Instance.Servers.Keys);
		server.sizeDelta = new UnityEngine.Vector2(80, server.sizeDelta.y);
		if(server.Find("DropDownButton/Text") is UnityEngine.RectTransform text)
			text.sizeDelta = new UnityEngine.Vector2(68, server.sizeDelta.y);
		server.localPosition = new UnityEngine.Vector3(0, 39.5f, 0);
		foreach(UnityEngine.Transform tr in server)
			tr.localPosition = new UnityEngine.Vector3(0, 0, 0);
		serverDropdown = server.GetComponent<ServerDropdown>();

		UnityEngine.RectTransform CreateButtonAt(float x, UnityEngine.GameObject from, string name, System.Action callback) {
			UnityEngine.RectTransform btn = UI.CreateButtonFrom(from, customServerEndPointText.transform, name, callback);
			btn.localPosition = new UnityEngine.Vector3(x, 39.5f, 0);
			return btn;
		}
		UnityEngine.RectTransform addButton = CreateButtonAt(-40, colorSchemeButton, "AddServer", () =>
			editServerViewController.TryPresent(mainFlowCoordinator.childFlowCoordinator, false));
		addButton.Find("Icon").GetComponent<HMUI.ImageView>().sprite = sprites.FirstOrDefault(sprite => sprite.name == "AddIcon");
		UnityEngine.RectTransform editButton = CreateButtonAt(52, colorSchemeButton, "EditServer", () =>
			editServerViewController.TryPresent(mainFlowCoordinator.childFlowCoordinator, true));
		editServerButton = editButton.GetComponent<UnityEngine.UI.Button>();
		(serverDropdown.addButton, serverDropdown.editButton) = (addButton.gameObject, editButton.gameObject);

		UnityEngine.RectTransform smallButton = (UnityEngine.RectTransform)colorSchemeButton.transform;
		UnityEngine.GameObject smallIcon = smallButton.Find("Icon").gameObject;
		UnityEngine.GameObject CreateShinyButton(float x, float scale, string name, UnityEngine.Sprite? sprite, System.Action callback) {
			UnityEngine.RectTransform btn = CreateButtonAt(x, okButton, name, callback);
			UnityEngine.Object.Destroy(btn.Find("Content").gameObject);
			UnityEngine.Vector2 sizeDelta = smallButton.sizeDelta;
			(btn.pivot, btn.sizeDelta) = (smallButton.pivot, new UnityEngine.Vector2(sizeDelta.x - 1, sizeDelta.y));
			UnityEngine.GameObject icon = UnityEngine.Object.Instantiate(smallIcon, btn);
			icon.transform.localScale = new UnityEngine.Vector3(scale, scale, 1);
			icon.GetComponent<HMUI.ImageView>().sprite = sprite;
			return btn.gameObject;
		}
		serverDropdown.removeButton = CreateShinyButton(-40.5f, .65f, "RemoveServer", sprites.FirstOrDefault(sprite => sprite.name == "CloseIcon"), () =>
			serverDropdown?.ClearEphemeral());
		serverDropdown.saveButton = CreateShinyButton(51.5f, .75f, "SaveServer", sprites.FirstOrDefault(sprite => sprite.name == "DownloadIcon"), () =>
			serverDropdown?.SaveEphemeral());

		BeatUpClient_Config.Instance.Servers.TryGetValue(customServerHostName.value, out string? statusUrl);
		SetNetworkConfig(customServerHostName, statusUrl);
	}
}
