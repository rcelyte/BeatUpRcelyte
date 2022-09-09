static partial class BeatUpClient {
	static UnityEngine.GameObject? prevLoading = null;
	static void ShowLoading(UnityEngine.GameObject? loading) {
		if(prevLoading != null)
			UnityEngine.Object.Destroy(prevLoading);
		prevLoading = loading;
		if(loading != null) {
			loading.name = "BeatUpClient_LoadingControl";
			loading.transform.localPosition = new UnityEngine.Vector3(0, 12, 0);
			loading.GetComponent<LoadingControl>().ShowLoading(Polyglot.Localization.Get("LABEL_CHECKING_SERVER_STATUS"));
		}
	}

	[Patch(PatchType.Prefix, typeof(HMUI.FlowCoordinator), "ProvideInitialViewControllers")]
	public static void FlowCoordinator_ProvideInitialViewControllers(HMUI.FlowCoordinator __instance, ref HMUI.ViewController mainViewController) {
		JoiningLobbyViewController? originalView = mainViewController as JoiningLobbyViewController;
		MultiplayerModeSelectionFlowCoordinator? flowCoordinator = __instance as MultiplayerModeSelectionFlowCoordinator;
		if(originalView != null && flowCoordinator != null) {
			mainViewController = flowCoordinator._multiplayerModeSelectionViewController;
			flowCoordinator._multiplayerModeSelectionViewController.transform.Find("Buttons")?.gameObject.SetActive(false);
			flowCoordinator._multiplayerModeSelectionViewController._maintenanceMessageText.gameObject.SetActive(false);
			flowCoordinator.showBackButton = true;
			ShowLoading(UnityEngine.Object.Instantiate(originalView._loadingControl.gameObject, mainViewController.transform));
		}
	}

	// This callback triggers twice if `_multiplayerStatusModel.GetMultiplayerStatusAsync()` was cancelled by pressing the back button
	[Patch(PatchType.Prefix, typeof(MainFlowCoordinator), nameof(MainFlowCoordinator.HandleMultiplayerModeSelectionFlowCoordinatorDidFinish))]
	public static bool MainFlowCoordinator_HandleMultiplayerModeSelectionFlowCoordinatorDidFinish(MultiplayerModeSelectionFlowCoordinator multiplayerModeSelectionFlowCoordinator) =>
		multiplayerModeSelectionFlowCoordinator.isActivated;

	// The UI deletes itself at the end of `MultiplayerModeSelectionFlowCoordinator.TryShowModeSelection()` without this
	[Patch.Overload(PatchType.Prefix, typeof(HMUI.FlowCoordinator), "ReplaceTopViewController", new[] {typeof(HMUI.ViewController), typeof(System.Action), typeof(HMUI.ViewController.AnimationType), typeof(HMUI.ViewController.AnimationDirection)})]
	public static bool FlowCoordinator_ReplaceTopViewController(HMUI.FlowCoordinator __instance, HMUI.ViewController viewController, System.Action finishedCallback, HMUI.ViewController.AnimationType animationType) {
		return (!(viewController is MultiplayerModeSelectionViewController)).Or(() => {
			__instance.TopViewControllerWillChange(viewController, viewController, animationType);
			finishedCallback();
		});
	}

	static string GetMaintenanceMessage(MultiplayerUnavailableReason reason, long? maintenanceWindowEndTime) {
		if(reason == MultiplayerUnavailableReason.MaintenanceMode)
			return Polyglot.Localization.GetFormat(MultiplayerUnavailableReasonMethods.LocalizedKey(reason), (TimeExtensions.AsUnixTime(maintenanceWindowEndTime.GetValueOrDefault()) - System.DateTime.UtcNow).ToString("h':'mm"));
		return $"{Polyglot.Localization.Get(MultiplayerUnavailableReasonMethods.LocalizedKey(reason))} ({MultiplayerUnavailableReasonMethods.ErrorCode(reason)})";
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerModeSelectionFlowCoordinator), nameof(MultiplayerModeSelectionFlowCoordinator.PresentMasterServerUnavailableErrorDialog))]
	public static bool MultiplayerModeSelectionFlowCoordinator_PresentMasterServerUnavailableErrorDialog(MultiplayerModeSelectionFlowCoordinator __instance, MultiplayerModeSelectionViewController ____multiplayerModeSelectionViewController, MultiplayerUnavailableReason reason, long? maintenanceWindowEndTime, string? remoteLocalizedMessage) {
		if(mainFlowCoordinator?.childFlowCoordinator != __instance)
			return false;
		ShowLoading(null);
		TMPro.TextMeshProUGUI message = ____multiplayerModeSelectionViewController._maintenanceMessageText;
		message.text = remoteLocalizedMessage ?? GetMaintenanceMessage(reason, maintenanceWindowEndTime);
		message.richText = true;
		message.transform.localPosition = new UnityEngine.Vector3(0, 15, 0);
		message.gameObject.SetActive(true);
		__instance.SetTitle(Polyglot.Localization.Get("LABEL_CONNECTION_ERROR"), HMUI.ViewController.AnimationType.In);
		return false;
	}

	static UnityEngine.GameObject[]? BeatUpServerUI = null;
	static UnityEngine.Sprite[] altCreateButtonSprites = new UnityEngine.Sprite[4];
	static bool createButtonSpriteState = false;
	[Patch(PatchType.Prefix, typeof(MultiplayerModeSelectionViewController), nameof(MultiplayerModeSelectionViewController.SetData))]
	public static void MultiplayerModeSelectionViewController_SetData(MultiplayerModeSelectionViewController __instance, MultiplayerStatusData? multiplayerStatusData, TMPro.TextMeshProUGUI ____maintenanceMessageText) {
		ShowLoading(null);
		____maintenanceMessageText.richText = false;
		____maintenanceMessageText.transform.localPosition = new UnityEngine.Vector3(0, -5, 0);
		bool isBeatUp = multiplayerStatusData?.minimumAppVersion?.EndsWith("b2147483647") == true;
		if(BeatUpServerUI != null)
			foreach(UnityEngine.GameObject element in BeatUpServerUI)
				element.SetActive(isBeatUp);
		UnityEngine.Transform? buttons = __instance.transform.Find("Buttons");
		if(buttons == null)
			return;
		if(createButtonSpriteState != isBeatUp) {
			HMUI.ButtonSpriteSwap? sprites = buttons.Find("CreateServerButton")?.GetComponent<HMUI.ButtonSpriteSwap>();
			if(sprites != null) {
				(sprites._normalStateSprite, altCreateButtonSprites[0]) = (altCreateButtonSprites[0], sprites._normalStateSprite);
				(sprites._highlightStateSprite, altCreateButtonSprites[1]) = (altCreateButtonSprites[1], sprites._highlightStateSprite);
				(sprites._pressedStateSprite, altCreateButtonSprites[2]) = (altCreateButtonSprites[2], sprites._pressedStateSprite);
				(sprites._disabledStateSprite, altCreateButtonSprites[3]) = (altCreateButtonSprites[3], sprites._disabledStateSprite);
				createButtonSpriteState = isBeatUp;
			}
		}
		buttons.gameObject.SetActive(true);
	}
}
