using static System.Linq.Enumerable;

static partial class BeatUpClient {
	static void ReenterModeSelection(MultiplayerModeSelectionFlowCoordinator? flowCoordinator) {
		if(flowCoordinator?._parentFlowCoordinator is MainFlowCoordinator mainFlowCoordinator) {
			mainFlowCoordinator.DismissFlowCoordinator(flowCoordinator, HMUI.ViewController.AnimationDirection.Horizontal, () => {
				mainFlowCoordinator.PresentMultiplayerModeSelectionFlowCoordinatorWithDisclaimerAndAvatarCreator();
			}, true);
		}
	}

	static void SelectorSetup() {
		if(Resolve<CustomNetworkConfig>() == null)
			return;
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

		MultiplayerModeSelectionViewController modeSelection = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerModeSelectionViewController>()[0];
		EditServerViewController.Instance ??= EditServerViewController.Create("BeatUpClient_EditServerView", modeSelection.transform.parent);
		EditServerViewController.Instance.mainFlowCoordinator = UnityEngine.Resources.FindObjectsOfTypeAll<MainFlowCoordinator>()[0];

		TMPro.TextMeshProUGUI customServerEndPointText = modeSelection._customServerEndPointText;
		customServerEndPointText.enabled = false;
		UnityEngine.RectTransform serverDropdown = ServerDropdown.Create(customServerEndPointText.transform, "Selector");
		serverDropdown.sizeDelta = new UnityEngine.Vector2(80, serverDropdown.sizeDelta.y);
		if(serverDropdown.Find("DropDownButton/Text") is UnityEngine.RectTransform text)
			text.sizeDelta = new UnityEngine.Vector2(68, serverDropdown.sizeDelta.y);
		serverDropdown.localPosition = new UnityEngine.Vector3(0, 39.5f, 0);
		foreach(UnityEngine.Transform tr in serverDropdown)
			tr.localPosition = new UnityEngine.Vector3(0, 0, 0);

		BeatUpClient_Config.Instance.Servers.TryGetValue(Resolve<SettingsManager>()!.settings.customServer.hostName, out string? statusUrl);
		UpdateNetworkConfig(Resolve<SettingsManager>()!.settings.customServer.hostName, statusUrl ?? string.Empty);
	}
}
