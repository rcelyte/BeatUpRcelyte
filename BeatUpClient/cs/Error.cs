static class BeatUpClient_Error {
	static string header = string.Empty, message = string.Empty;
	static bool skip = false;
	[BeatUpClient.Detour(typeof(MainFlowCoordinator), nameof(MainFlowCoordinator.HandleMainMenuViewControllerDidFinish))]
	static void MainFlowCoordinator_HandleMainMenuViewControllerDidFinish(MainFlowCoordinator self, MainMenuViewController viewController, MainMenuViewController.MenuButton subMenuType) {
		if(skip || subMenuType != MainMenuViewController.MenuButton.Multiplayer) {
			BeatUpClient.Base(self, viewController, subMenuType);
			return;
		}
		self._simpleDialogPromptViewController.Init(header, message, BGLib.Polyglot.Localization.Get("BUTTON_OK"), buttonNumber => {
			skip = true;
			self.DismissViewController(self._simpleDialogPromptViewController, HMUI.ViewController.AnimationDirection.Vertical, () =>
				self.HandleMainMenuViewControllerDidFinish(viewController, MainMenuViewController.MenuButton.Multiplayer));
		});
		self.PresentViewController(self._simpleDialogPromptViewController);
	}
	public static void Init(string header, string message) {
		(BeatUpClient_Error.header, BeatUpClient_Error.message) = (header, message);
		uint patchCount = 0;
		BeatUpClient.GatherMethods(typeof(BeatUpClient_Error), ref patchCount)();
	}
}
