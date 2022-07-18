static partial class BeatUpClient {
	class InstantHoverHint : HMUI.HoverHint {
		public override void OnPointerEnter(UnityEngine.EventSystems.PointerEventData eventData) {
			_hoverHintController._isHiding = false;
			_hoverHintController.StopAllCoroutines();
			_hoverHintController.SetupAndShowHintPanel(this);
		}
	}

	static void AddHoverHint(UnityEngine.GameObject element, string key) {
		element.AddComponent<HMUI.Touchable>();
		LocalizedHoverHint hint = element.AddComponent<LocalizedHoverHint>();
		hint.localizedComponent = element.AddComponent<InstantHoverHint>();
		hint.Key = key;
	}

	static void LobbyUISetup() {
		UnityEngine.Transform MultiplayerSettingsPanel = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerSettingsPanelController>()[0].transform;
		UnityEngine.Transform enableDownloads = UI.CreateToggle(MultiplayerSettingsPanel, "EnableDownloads", "BEATUP_DIRECT_DOWNLOADS", new Property<bool>(BeatUpClient_Config.Instance, nameof(BeatUpClient_Config.DirectDownloads)));
		enableDownloads.GetComponentInChildren<HMUI.CurvedTextMeshPro>().richText = true;
		AddHoverHint(enableDownloads.gameObject, "BEATUP_ENABLE_DIRECT_DOWNLOADS");
		if(!haveMpEx) {
			UnityEngine.Transform hideLevels = UI.CreateToggle(MultiplayerSettingsPanel, "HideLevels", "BEATUP_HIDE_OTHER_LEVELS", new Property<bool>(BeatUpClient_Config.Instance, nameof(BeatUpClient_Config.HideOtherLevels)));
			AddHoverHint(hideLevels.gameObject, "BEATUP_MAY_IMPROVE_PERFORMANCE");
		}
		infoText = UnityEngine.Object.Instantiate(MultiplayerSettingsPanel.parent.Find("PlayerOptions/ViewPort/Content/SinglePlayerOnlyTitle").gameObject, MultiplayerSettingsPanel);
		infoText.name = "BeatUpClient_Info";
		Polyglot.LocalizedTextMeshProUGUI text = infoText.GetComponentInChildren<Polyglot.LocalizedTextMeshProUGUI>();
		text.localizedComponent.richText = true;
		text.Key = "BEATUP_INFO";

		DifficultyPanel.Init();
		lobbyDifficultyPanel = new DifficultyPanel(UnityEngine.Resources.FindObjectsOfTypeAll<LobbySetupViewController>()[0].transform.GetChild(0), 2, 90);
	}
}
