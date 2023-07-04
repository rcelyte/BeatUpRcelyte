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

	[Detour(typeof(HMUI.FlowCoordinator), nameof(HMUI.FlowCoordinator.ProvideInitialViewControllers))]
	static void FlowCoordinator_ProvideInitialViewControllers(HMUI.FlowCoordinator self, HMUI.ViewController mainViewController, HMUI.ViewController leftScreenViewController, HMUI.ViewController rightScreenViewController, HMUI.ViewController bottomScreenViewController, HMUI.ViewController topScreenViewController) {
		JoiningLobbyViewController? originalView = mainViewController as JoiningLobbyViewController;
		MultiplayerModeSelectionFlowCoordinator? flowCoordinator = self as MultiplayerModeSelectionFlowCoordinator;
		if(originalView != null && flowCoordinator != null) {
			mainViewController = flowCoordinator._multiplayerModeSelectionViewController;
			flowCoordinator._multiplayerModeSelectionViewController.transform.Find("Buttons")?.gameObject.SetActive(false);
			flowCoordinator._multiplayerModeSelectionViewController._maintenanceMessageText.gameObject.SetActive(false);
			flowCoordinator.showBackButton = true;
			ShowLoading(UnityEngine.Object.Instantiate(originalView._loadingControl.gameObject, mainViewController.transform));
		}
		Base(self, mainViewController, leftScreenViewController, rightScreenViewController, bottomScreenViewController, topScreenViewController);
	}

	// This callback triggers twice if `_multiplayerStatusModel.GetMultiplayerStatusAsync()` was cancelled by pressing the back button
	[Detour(typeof(MainFlowCoordinator), nameof(MainFlowCoordinator.HandleMultiplayerModeSelectionFlowCoordinatorDidFinish))]
	static void MainFlowCoordinator_HandleMultiplayerModeSelectionFlowCoordinatorDidFinish(MainFlowCoordinator self, MultiplayerModeSelectionFlowCoordinator multiplayerModeSelectionFlowCoordinator) {
		if(multiplayerModeSelectionFlowCoordinator.isActivated)
			Base(self, multiplayerModeSelectionFlowCoordinator);
	}

	// The UI deletes itself at the end of `MultiplayerModeSelectionFlowCoordinator.TryShowModeSelection()` without this
	[Detour(typeof(HMUI.FlowCoordinator), nameof(HMUI.FlowCoordinator.ReplaceTopViewController))]
	static void FlowCoordinator_ReplaceTopViewController(HMUI.FlowCoordinator self, HMUI.ViewController viewController, System.Action finishedCallback, HMUI.ViewController.AnimationType animationType, HMUI.ViewController.AnimationDirection animationDirection) {
		if(viewController is MultiplayerModeSelectionViewController) {
			self.TopViewControllerWillChange(viewController, viewController, animationType);
			finishedCallback();
		} else {
			Base(self, viewController, finishedCallback, animationType, animationDirection);
		}
	}

	static string GetMaintenanceMessage(MultiplayerUnavailableReason reason, long? maintenanceWindowEndTime) {
		if(reason == MultiplayerUnavailableReason.MaintenanceMode)
			return Polyglot.Localization.GetFormat(MultiplayerUnavailableReasonMethods.LocalizedKey(reason), (TimeExtensions.AsUnixTime(maintenanceWindowEndTime.GetValueOrDefault()) - System.DateTime.UtcNow).ToString("h':'mm"));
		return $"{Polyglot.Localization.Get(MultiplayerUnavailableReasonMethods.LocalizedKey(reason))} ({MultiplayerUnavailableReasonMethods.ErrorCode(reason)})";
	}

	[Detour(typeof(MultiplayerModeSelectionFlowCoordinator), nameof(MultiplayerModeSelectionFlowCoordinator.PresentMasterServerUnavailableErrorDialog))]
	static void MultiplayerModeSelectionFlowCoordinator_PresentMasterServerUnavailableErrorDialog(MultiplayerModeSelectionFlowCoordinator self, MultiplayerUnavailableReason reason, System.Exception exception, long? maintenanceWindowEndTime, string? remoteLocalizedMessage) {
		if(!(self._parentFlowCoordinator is MainFlowCoordinator))
			return;
		ShowLoading(null);
		TMPro.TextMeshProUGUI message = self._multiplayerModeSelectionViewController._maintenanceMessageText;
		message.text = remoteLocalizedMessage ?? GetMaintenanceMessage(reason, maintenanceWindowEndTime);
		message.richText = true;
		message.transform.localPosition = new UnityEngine.Vector3(0, 15, 0);
		message.gameObject.SetActive(true);
		self.SetTitle(Polyglot.Localization.Get("LABEL_CONNECTION_ERROR"), HMUI.ViewController.AnimationType.In);
	}

	static UnityEngine.GameObject[]? BeatUpServerUI = null;
	static (UnityEngine.Sprite normal, UnityEngine.Sprite highlight, UnityEngine.Sprite pressed, UnityEngine.Sprite disabled) createButtonSprites, heartButtonSprites;
	static HMUI.ButtonSpriteSwap? spriteSwap = null;
	[Detour(typeof(MultiplayerModeSelectionViewController), nameof(MultiplayerModeSelectionViewController.SetData))]
	static void MultiplayerModeSelectionViewController_SetData(MultiplayerModeSelectionViewController self, MultiplayerStatusData? multiplayerStatusData) {
		ShowLoading(null);
		self._maintenanceMessageText.richText = false;
		self._maintenanceMessageText.transform.localPosition = new UnityEngine.Vector3(0, -5, 0);
		bool isBeatUp = multiplayerStatusData?.minimumAppVersion?.EndsWith("b2147483647") == true;
		if(BeatUpServerUI != null)
			foreach(UnityEngine.GameObject element in BeatUpServerUI)
				element.SetActive(isBeatUp);
		UnityEngine.Transform? createButton = self.transform.Find("Buttons/CreateServerButton");
		if(createButton != null) {
			if(spriteSwap != null)
				(spriteSwap._normalStateSprite, spriteSwap._highlightStateSprite, spriteSwap._pressedStateSprite, spriteSwap._disabledStateSprite) =
					isBeatUp ? heartButtonSprites : createButtonSprites;
			createButton.parent.gameObject.SetActive(true);
		}
		Base(self, multiplayerStatusData);
	}
}
